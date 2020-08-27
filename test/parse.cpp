#include <kaleidoscope/parser.hpp>

using namespace kaleidoscope;

int main() {
    Parser parser{Tokenizer{std::cin}};

    parser.setDefHandler([](auto &) {
        std::cout << "parsed a function definition" << std::endl;
    });

    parser.setExternHandler([](auto &) {
        std::cout << "parsed an external" << std::endl;
    });

    parser.setTopLevelHandler([](auto &) {
        std::cout << "parsed a top level expression" << std::endl;
    });

    std::cout << "ready> ";
    while (parser.parse()) {
        std::cout << "ready> ";
    }

}
