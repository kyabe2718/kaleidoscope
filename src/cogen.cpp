#include <kaleidoscope/ast.hpp>

#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <iostream>

namespace kaleidoscope
{

template <class... Msg>
llvm::Value* logErrorV(Msg&&... msg)
{
    ((std::cerr << "LogErrorV: ") << ... << msg) << std::endl;
    return nullptr;
}

template <class... Msg>
llvm::Function* logErrorF(Msg&&... msg)
{
    ((std::cerr << "LogFunctionV: ") << ... << msg) << std::endl;
    ;
    return nullptr;
}

llvm::Value* NumberExpAST::codegen(CodeGenEnv& env)
{
    return llvm::ConstantFP::get(env.context, llvm::APFloat(val));
}

llvm::Value* VariableExprAST::codegen(CodeGenEnv& env)
{
    auto v = env.named_value[name];
    if (!v)
        return logErrorV("unknown variable name: ", name);
    return v;
}

llvm::Value* BinaryExprAST::codegen(CodeGenEnv& env)
{
    auto l = lhs->codegen(env);
    auto r = rhs->codegen(env);
    if (!l || !r)
        return nullptr;

    if (op == "+") {
        return env.builder.CreateFAdd(l, r, "addtmp");  // addtmpは単なる名付けのヒント
    } else if (op == "-") {
        return env.builder.CreateFSub(l, r, "subtmp");
    } else if (op == "*") {
        return env.builder.CreateFMul(l, r, "multmp");
    } else if (op == "<") {
        auto ret = env.builder.CreateFCmpULT(l, r, "ulttmp");  // unordered less than. unordered: quiet_nanが入りうる
        // bool -> double
        return env.builder.CreateUIToFP(ret, llvm::Type::getDoubleTy(env.context), "booltmp");
    } else if (op == ">") {
        auto ret = env.builder.CreateFCmpUGT(l, r, "ugttmp");
        // bool -> double
        return env.builder.CreateUIToFP(ret, llvm::Type::getDoubleTy(env.context), "booltmp");
    } else {
        return logErrorV("unknown binary operator: ", op);
    }
}

llvm::Value* CallExprAST::codegen(CodeGenEnv& env)
{
    //    auto func = env.module->getFunction(callee);
    auto func = env.getFunction(callee);  // module内に，proto type宣言しかなくてもよい

    if (!func)
        return logErrorV("unknown function referenced: ", callee);

    if (func->arg_size() != args.size())
        return logErrorV("argument mismatch: ", callee);

    std::vector<llvm::Value*> arg_values;
    for (auto& arg : args) {
        arg_values.push_back(arg->codegen(env));
    }

    return env.builder.CreateCall(func, arg_values, "calltmp");
}

llvm::Function* PrototypeAST::codegen(CodeGenEnv& env)
{
    // 型は全てdouble
    std::vector<llvm::Type*> doubles(args.size(), llvm::Type::getDoubleTy(env.context));

    // create function type
    auto func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(env.context), doubles, false);

    // create the IR function corresponding to the prototype
    auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, name, env.module.get());

    // This phase isn't strictly necessary
    // keep the name consistent to make IR readable
    size_t id = 0;
    for (auto& farg : func->args()) {
        farg.setName(args.at(id++));
    }

    return func;
}

llvm::Function* FunctionAST::codegen(CodeGenEnv& env)
{
    std::unique_ptr<PrototypeAST> p = nullptr;
    proto.swap(p);
    auto name = p->getName();
    auto& proto = env.proto_func.insert_or_assign(name, std::move(p)).first->second;

    auto func = env.getFunction(name);

    if (!func)
        func = proto->codegen(env);

    if (!func)
        return logErrorF("unknown error in prototype");

    if (!func->empty())
        return logErrorF("function cannot be redefined");

    // 1入力1出力で、内部に分岐を含まないコードブロック
    // フローチャートにおいて、1つのノードで表される
    auto bb = llvm::BasicBlock::Create(env.context, "entry", func);
    env.builder.SetInsertPoint(bb);

    // 変数のマッピングを更新
    env.named_value.clear();
    for (auto& arg : func->args())
        env.named_value[arg.getName()] = &arg;

    if (auto retval = body->codegen(env)) {
        env.builder.CreateRet(retval);  // finish off the function
        llvm::verifyFunction(*func);
        env.FPM->run(*func);
        return func;
    } else {
        func->eraseFromParent();
        return nullptr;
    }
}
}  // namespace kaleidoscope
