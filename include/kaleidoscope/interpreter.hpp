#pragma once

#include "parser.hpp"

#include <llvm/Support/TargetSelect.h>

namespace kaleidoscope
{

struct InterpreterBase
{
protected:
    InterpreterBase()
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
    }
};

struct Interpreter : InterpreterBase
{
    struct Config
    {
        bool print_ir;
    };

    template <class T>
    explicit Interpreter(T&& input, const std::string& mod_name,
        Config config = Config{.print_ir = false})
        : InterpreterBase(),
          parser{Parser{Tokenizer{std::forward<T>(input)}}},
          env{mod_name},
          config{config}
    {
        initialize();
    }

    bool run();

private:
    void initialize();

    Parser parser;
    CodeGenEnv env;
    Config config;
};

}  // namespace kaleidoscope