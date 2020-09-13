#include <kaleidoscope/interpreter.hpp>

namespace kaleidoscope
{
void Interpreter::initialize()
{
    parser.setDefHandler([&](std::unique_ptr<FunctionAST> def) {
        if (auto* code = def->codegen(env)) {
            if (config.print_ir) {
                llvm::outs() << "; parsed a function definition\n";
                llvm::outs() << *code << '\n';
            }
            env.JIT->addModule(std::move(env.module));
            env.initModAndPassManager("my cool jit");
        } else {
            std::cerr << "[function def] failed to cogen" << std::endl;
        }
    });

    parser.setExternHandler([&](std::unique_ptr<PrototypeAST> ext) {
        if (auto* code = ext->codegen(env)) {
            if (config.print_ir) {
                llvm::outs() << "; parsed an external\n";
                llvm::outs() << *code << '\n';
            }
            env.proto_func[ext->getName()] = std::move(ext);
        } else {
            std::cerr << "[external] failed to cogen" << std::endl;
        }
    });

    parser.setTopLevelHandler([&](std::unique_ptr<FunctionAST> top) {
        if (auto* code = top->codegen(env)) {
            if (config.print_ir) {
                llvm::outs() << "; parsed a top-level\n";
                llvm::outs() << *code << '\n';
            }
            auto H = env.JIT->addModule(std::move(env.module));
            env.initModAndPassManager("mod");

            auto ExprSymbol = env.JIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "__anon_expr is not found.");

            double (*fp)() = reinterpret_cast<double (*)()>(static_cast<intptr_t>(*ExprSymbol.getAddress()));

            std::cout << "Evaluated to " << fp() << std::endl;

            env.JIT->removeModule(H);
        } else {
            std::cerr << "[top level] failed to cogen" << std::endl;
        }
    });
}

bool Interpreter::run()
{
    return parser.parse();
}
}  // namespace kaleidoscope