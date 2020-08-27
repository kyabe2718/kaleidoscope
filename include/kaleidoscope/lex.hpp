#pragma once

#include <string>
#include <ostream>
#include <vector>
#include <variant>
#include <optional>

#include <boost/hana/functional/overload.hpp>

namespace kaleidoscope {
namespace token {

struct identifier {
    std::string str;
};

struct keyword {
    std::string str;

    static inline std::vector<std::string> list = {
            "def", "extern", "if", "else"
    };
};

struct punctuator {
    std::string str;

    static inline std::vector<std::string> list = {
            "+", "-", "*", "/", "<", ">", "=", "(", ")", ";", ","
    };
};

struct number {
    double num;
};

struct eof {
};

struct unknown {
    std::string str;
};
}

using Token = std::variant<
        token::eof,
        token::keyword,
        token::punctuator,
        token::identifier,
        token::number,
        token::unknown
>;

inline std::ostream &operator<<(std::ostream &os, const Token &token) {
    std::visit(
            boost::hana::overload(
                    [&](const token::eof &) { os << "eof"; },
                    [&](const token::keyword &kw) { os << "keyword: " << kw.str; },
                    [&](const token::punctuator &p) { os << "punctuator: " << p.str; },
                    [&](const token::identifier &id) { os << "identifier: " << id.str; },
                    [&](const token::number &num) { os << "number: " << num.num; },
                    [&](const token::unknown &uk) { os << "unknown: " << uk.str; }
            ), token);
    return os;
}

struct Tokenizer {
    explicit Tokenizer(std::istream &iss) : iss{iss} {}
    Tokenizer(const Tokenizer &) = delete;
    Tokenizer(Tokenizer &&) = default;

    const Token &curToken() const { return curTok; }
    const Token &getNextToken() { return curTok = getToken(); }

private:
    Token getToken();

    std::istream &iss;
    Token curTok;
};

namespace token {
inline bool is_def(const Token &token) {
    return std::holds_alternative<keyword>(token) && (std::get<keyword>(token).str == "def");
}

inline bool is_extern(const Token &token) {
    return std::holds_alternative<keyword>(token) && (std::get<keyword>(token).str == "extern");
}

inline bool is_eof(const Token &token) {
    return std::holds_alternative<eof>(token);
}

inline bool is_identifier(const Token &token) {
    return std::holds_alternative<identifier>(token);
}

inline bool is_num(const Token &token) {
    return std::holds_alternative<number>(token);
}

inline bool is_punc(const Token &token) {
    return std::holds_alternative<punctuator>(token);
}

inline bool is_l_paren(const Token &token) {
    return std::holds_alternative<punctuator>(token) && (std::get<punctuator>(token).str == "(");
}

inline bool is_r_paren(const Token &token) {
    return std::holds_alternative<punctuator>(token) && (std::get<punctuator>(token).str == ")");
}

inline bool is_comma(const Token &token) {
    return std::holds_alternative<punctuator>(token) && (std::get<punctuator>(token).str == ",");
}

inline bool is_semicolon(const Token &token) {
    return std::holds_alternative<punctuator>(token) && (std::get<punctuator>(token).str == ";");
}

inline std::string get_identifier(const Token &token) {
    if (is_identifier(token))
        return std::get<identifier>(token).str;
    else
        return "";
}

inline std::optional<double> get_num(const Token &token) {
    if (is_num(token))
        return std::get<number>(token).num;
    else
        return std::nullopt;
}

inline std::string get_punc(const Token &token) {
    if (is_punc(token))
        return std::get<punctuator>(token).str;
    else
        return "";
}
}
}