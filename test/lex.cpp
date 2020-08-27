#include <iostream>

#include <kaleidoscope/lex.hpp>

int main() {

    using namespace kaleidoscope;

    Tokenizer tok{std::cin};

    while (
            std::visit(boost::hana::overload(
                    [](const token::eof &t) {
                        std::cout << "eof" << std::endl;
                        return false;
                    },
                    [](const token::keyword &t) {
                        std::cout << "keyword: " << t.str << std::endl;
                        return true;
                    },
                    [](const token::punctuator &t) {
                        std::cout << "punc: " << t.str << std::endl;
                        return true;
                    },
                    [](const token::identifier &t) {
                        std::cout << "identifier: " << t.str << std::endl;
                        return true;
                    },
                    [](const token::number &t) {
                        std::cout << "number: " << t.num << std::endl;
                        return true;
                    },
                    [](const token::unknown &t) {
                        std::cout << "unknown: " << t.str << std::endl;
                        return false;
                    }
            ), tok.getNextToken()));

    return 0;
}
