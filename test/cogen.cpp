#include <kaleidoscope/parser.hpp>

#include <fstream>
#include <iostream>
#include <memory>

using namespace kaleidoscope;

int main(int argc, char** argv)
{

    bool is_finput = (argc > 1);

    auto parser = [&] {
        if (is_finput) {
            return Parser{Tokenizer{argv[1]}};
        } else {
            return Parser{Tokenizer{std::cin}};
        }
    }();

    CodeGenEnv env{"my cool jit"};

    parser.setDefHandler([&](const FunctionAST& def) {
        if (auto code = def.codegen(env)) {
            llvm::outs() << "; parsed a function definition\n";
            llvm::outs() << *code << '\n';
        } else {
            std::cerr << "[function def] failed to cogen" << std::endl;
        }
    });

    parser.setExternHandler([&](const PrototypeAST& ext) {
        if (auto code = ext.codegen(env)) {
            llvm::outs() << "; parsed an external\n";
            llvm::outs() << *code << '\n';
        } else {
            std::cerr << "[external] failed to cogen" << std::endl;
        }
    });

    parser.setTopLevelHandler([&](const FunctionAST& top) {
        if (auto code = top.codegen(env)) {
            llvm::outs() << "; parsed a top level expression\n";
            llvm::outs() << *code << '\n';
        } else {
            std::cerr << "[top level] failed to cogen" << std::endl;
        }
    });

    if (is_finput) {
        while (parser.parse())
            ;
    } else {
        std::cout << "ready> ";
        while (parser.parse())
            std::cout << "\nready> ";
    }
}
