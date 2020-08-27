#include <kaleidoscope/parser.hpp>

#include <llvm/Support/raw_ostream.h>

using namespace kaleidoscope;

int main() {
    Parser parser{Tokenizer{std::cin}};

    CodeGenEnv env{"my cool jit"};

    parser.setDefHandler([&](const FunctionAST &def) {
        std::cerr << "parsed a function definition" << std::endl;
        if (auto code = def.codegen(env)) {
            code->print(llvm::errs());
            llvm::errs() << '\n';
            llvm::errs().flush();
        } else {
            std::cerr << "failed to cogen" << std::endl;
        }
    });

    parser.setExternHandler([&](const PrototypeAST &ext) {
        std::cerr << "parsed an external" << std::endl;
        if (auto code = ext.codegen(env)) {
            code->print(llvm::errs());
            llvm::errs() << '\n';
            llvm::errs().flush();
        } else {
            std::cerr << "failed to cogen" << std::endl;
        }
    });

    parser.setTopLevelHandler([&](const FunctionAST &top) {
        std::cerr << "parsed a top level expression" << std::endl;
        if (auto code = top.codegen(env)) {
            code->print(llvm::errs());
            llvm::errs() << '\n';
            llvm::errs().flush();
        } else {
            std::cerr << "failed to cogen" << std::endl;
        }
    });

    std::cout << "ready> ";
    while (parser.parse()) {
        std::cout << std::endl << "ready> ";
    }

}
