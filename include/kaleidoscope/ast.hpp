#pragma once

#include <string>
#include <memory>

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>

#include <kaleidoscope/KaleidoscopeJIT.h>

namespace kaleidoscope
{

struct CodeGenEnv;

struct ExprAST
{
    virtual ~ExprAST() = default;
    virtual llvm::Value* codegen(CodeGenEnv&) = 0;  // llvm::ValueはSSAでのレジスタを表す
};

struct NumberExpAST : ExprAST
{
    explicit NumberExpAST(double val) : val{val} {}

    llvm::Value* codegen(CodeGenEnv&) override;

private:
    double val;
};

struct VariableExprAST : ExprAST
{
    explicit VariableExprAST(std::string name) : name{std::move(name)} {}

    llvm::Value* codegen(CodeGenEnv&) override;

private:
    std::string name;
};

struct BinaryExprAST : ExprAST
{
    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op{std::move(op)}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    llvm::Value* codegen(CodeGenEnv&) override;

private:
    std::string op;
    std::unique_ptr<ExprAST> lhs, rhs;
};

struct CallExprAST : ExprAST
{
    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee{std::move(callee)}, args{std::move(args)} {}

    llvm::Value* codegen(CodeGenEnv&) override;

private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
};

//  関数の宣言
struct PrototypeAST
{
    PrototypeAST(std::string name, std::vector<std::string> args)
        : name{std::move(name)}, args{std::move(args)} {}

    [[nodiscard]] std::string& getName() { return name; }

    llvm::Function* codegen(CodeGenEnv&);

private:
    std::string name;
    std::vector<std::string> args;
};

struct FunctionAST
{
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
        : proto{std::move(proto)}, body{std::move(body)} {}

    llvm::Function* codegen(CodeGenEnv&);

private:
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;
};

struct CodeGenEnv
{

    explicit CodeGenEnv(const std::string& mod_name)
        : builder(context), JIT{std::make_unique<llvm::orc::KaleidoscopeJIT>()}
    {
        initModAndPassManager(mod_name);
    }

    void initModAndPassManager(const std::string& mod_name)
    {
        module = std::make_unique<llvm::Module>(mod_name, context);
        FPM = std::make_unique<llvm::legacy::FunctionPassManager>(module.get());

        module->setDataLayout(JIT->getTargetMachine().createDataLayout());

        FPM->add(llvm::createInstructionCombiningPass());
        FPM->add(llvm::createReassociatePass());
        FPM->add(llvm::createNewGVNPass());
        FPM->add(llvm::createCFGSimplificationPass());
        FPM->doInitialization();
    }

    llvm::Function* getFunction(const std::string& name)
    {
        if (auto* f = module->getFunction(name))
            return f;

        if (auto fi = proto_func.find(name); fi != proto_func.end())
            return fi->second->codegen(*this);

        return nullptr;
    }

    llvm::LLVMContext context;  // llvmのいろいろ
    llvm::IRBuilder<> builder;
    std::unordered_map<std::string, llvm::Value*> named_value;
    std::unordered_map<std::string, std::unique_ptr<PrototypeAST>> proto_func;

    std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT;

    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::legacy::FunctionPassManager> FPM;
};

}  // namespace kaleidoscope