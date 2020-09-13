#include <kaleidoscope/interpreter.hpp>

#include <iostream>

using namespace kaleidoscope;

int main(int argc, char** argv)
{

    bool is_finput = (argc > 1);

    auto interpreter = [&] {
        if (is_finput) {
            return Interpreter{argv[1], "my-cool-jit"};
        } else {
            return Interpreter{std::cin, "my-cool-jit"};
        }
    }();


    if (is_finput) {
        while (interpreter.run())
            ;
    } else {
        std::cout << "ready> ";
        while (interpreter.run())
            std::cout << std::flush << "\nready> ";
    }
}
