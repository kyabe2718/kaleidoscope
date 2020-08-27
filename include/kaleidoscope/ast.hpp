#pragma once

#include <string>
#include <memory>

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace kaleidoscope {

struct CodeGenEnv;

struct ExprAST {
    virtual ~ExprAST() = default;
    virtual llvm::Value *codegen(CodeGenEnv &) const = 0;     // llvm::ValueはSSAでのレジスタを表す
};

struct NumberExpAST : ExprAST {
    explicit NumberExpAST(double val) : val{val} {}

    llvm::Value *codegen(CodeGenEnv &) const override;
private:
    double val;
};

struct VariableExprAST : ExprAST {
    explicit VariableExprAST(std::string name) : name{std::move(name)} {}

    llvm::Value *codegen(CodeGenEnv &) const override;
private:
    std::string name;
};

struct BinaryExprAST : ExprAST {
    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
            : op{std::move(op)}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    llvm::Value *codegen(CodeGenEnv &) const override;
private:
    std::string op;
    std::unique_ptr<ExprAST> lhs, rhs;
};

struct CallExprAST : ExprAST {
    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
            : callee{std::move(callee)}, args{std::move(args)} {}

    llvm::Value *codegen(CodeGenEnv &) const override;
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
};

//  関数の宣言
struct PrototypeAST {
    PrototypeAST(std::string name, std::vector<std::string> args)
            : name{std::move(name)}, args{std::move(args)} {}

    [[nodiscard]] const std::string &getName() const { return name; }

    llvm::Function *codegen(CodeGenEnv &) const;

private:
    std::string name;
    std::vector<std::string> args;
};

struct FunctionAST {
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
            : proto{std::move(proto)}, body{std::move(body)} {}

    llvm::Function *codegen(CodeGenEnv &) const;
private:
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;
};

struct CodeGenEnv {

    explicit CodeGenEnv(const std::string &mod_name)
            : builder(context),
              module{std::make_unique<llvm::Module>(mod_name, context)} {}

    llvm::LLVMContext context;  // llvmのいろいろ
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> module;
    std::unordered_map<std::string, llvm::Value *> named_value;
};

}