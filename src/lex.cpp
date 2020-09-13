#include <kaleidoscope/lex.hpp>

#include <iostream>
#include <optional>

namespace kaleidoscope
{

Token Tokenizer::getToken()
{

    // consume space
    while (std::isspace(iss->peek())) {
        iss->ignore();
    }

    // comment
    if (iss->peek() == '#') {
        while (iss->peek() != EOF && iss->peek() != '\n' && iss->peek() != '\r')
            iss->ignore();

        if (iss->get() != EOF) {  // consume \n or \r
            return getToken();
        }
    }

    // identifier/keyword: [a-Z][a-Z0-9]*
    if (std::isalpha(iss->peek())) {
        std::string id;
        id = static_cast<char>(iss->get());

        while (std::isalnum(iss->peek()) || iss->peek() == '_')
            id += static_cast<char>(iss->get());

        // Is id keyword ?
        const auto& ls = token::keyword::list;
        if (auto p = std::find(ls.begin(), ls.end(), id); p != ls.end())
            return {token::keyword{id}};

        return {token::identifier{id}};
    }

    // num: [0-9]*(.[0-9]*)?
    if (std::isdigit(iss->peek())) {
        std::string num;
        num = static_cast<char>(iss->get());

        while (std::isdigit(iss->peek()) || iss->peek() == '.')
            num += static_cast<char>(iss->get());

        try {
            return {token::number{std::stod(num)}};
        } catch (...) {
            return {token::unknown{num}};
        }
    }

    // punctuator
    for (const auto& punc : token::punctuator::list) {
        char buf[10] = {};
        iss->read(buf, punc.length());
        if (punc == buf) {
            return {token::punctuator{punc}};
        } else {
            for (size_t i = 0; i < punc.length(); ++i) {
                iss->unget();
            }
        }
    }

    if (iss->peek() == EOF) {
        return {token::eof{}};
    }

    char c = iss->get();
    return {token::unknown{{c}}};
}

}  // namespace kaleidoscope
