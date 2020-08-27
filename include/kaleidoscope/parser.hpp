#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <functional>

#include "lex.hpp"
#include "ast.hpp"

namespace kaleidoscope {

struct ParserImpl;

struct Parser {
    explicit Parser(Tokenizer &&tok);

    bool parse();

    void setDefHandler(std::function<void(const FunctionAST &)> handler) {
        def_handler = std::move(handler);
    }

    void setExternHandler(std::function<void(const PrototypeAST &)> handler) {
        extern_handler = std::move(handler);
    }

    void setTopLevelHandler(std::function<void(const FunctionAST &)> handler) {
        toplevel_handler = std::move(handler);
    }

private:
    // Tokenizer tokenizer;
    std::shared_ptr<ParserImpl> impl;
    std::function<void(const FunctionAST &)> def_handler;
    std::function<void(const PrototypeAST &)> extern_handler;
    std::function<void(const FunctionAST &)> toplevel_handler;
};
}