#include <iostream>
#include <string>


#include <kaleidoscope/lex.hpp>
#include <kaleidoscope/parser.hpp>


int main() {

    std::cout << "kaleidoscope" << std::endl;

    using namespace kaleidoscope;

    Token token;
    while ((token = gettok()) != Token::eof) {
        switch (token) {
            case Token::def:
                std::cout << "def" << std::endl;
                break;
            case Token::extrn:
                std::cout << "extern" << std::endl;
                break;
            case Token::identifier:
                std::cout << "identifier: " << IdentifierStr << std::endl;
                break;
            case Token::number:
                std::cout << "number: " << NumVal << std::endl;
                break;
            case Token::unknown:
                std::cout << "unknown: " << IdentifierStr << std::endl;
                break;
            default:
                return 0;
        }
    }

    return 0;
}
