#include <kaleidoscope/parser.hpp>

#include <unordered_map>

namespace kaleidoscope {

struct ParserImpl {
    template<class... Msgs>
    std::unique_ptr<ExprAST> logErrorE(Msgs &&... msgs) {
        ((std::cerr << "logErrorE: ") << ...  << std::forward<Msgs>(msgs)) << std::endl;
        return nullptr;
    }

    template<class... Msgs>
    std::unique_ptr<PrototypeAST> logErrorP(Msgs &&... msgs) {
        ((std::cerr << "logErrorP: ") << ...  << std::forward<Msgs>(msgs)) << std::endl;
        return nullptr;
    }

    std::unique_ptr<ExprAST> parseIdentifier() {
        if (auto id = token::get_identifier(tokenizer.curToken()); !id.empty()) {
            auto res = std::make_unique<VariableExprAST>(std::move(id));
            tokenizer.getNextToken(); // consume itself
            return res;
        }
        return logErrorE("expected identifier in identifier");
    }

    std::unique_ptr<ExprAST> parseNumExpr() {
        if (auto num = token::get_num(tokenizer.curToken()); num.has_value()) {
            auto res = std::make_unique<NumberExpAST>(num.value());
            tokenizer.getNextToken();
            return res;
        }
        return logErrorE("expected number in number-expr");
    }

    std::unique_ptr<ExprAST> parseParenExpr() {
        if (!token::is_l_paren(tokenizer.curToken()))
            return logErrorE("expected '(' in paren-expr");
        tokenizer.getNextToken(); // consume '('

        auto expr = parseExpression();
        if (!expr)
            return nullptr;

        if (!token::is_r_paren(tokenizer.curToken()))
            return logErrorE("expected ')' in paren-expr");

        tokenizer.getNextToken(); // consume ')'
        return expr;
    }

    // identifier-expr ::= identifier | identifier '(' (expression (',' expression)*)? ')'
    std::unique_ptr<ExprAST> parseIdentifierExpr() {
        std::string id_name = token::get_identifier(tokenizer.curToken());
        if (id_name.empty())
            return logErrorE("expected identifier in identifier-expr");

        if (!token::is_l_paren(tokenizer.getNextToken()))
            return std::make_unique<VariableExprAST>(std::move(id_name));

        // call
        std::vector<std::unique_ptr<ExprAST>> args;
        if (!token::is_r_paren(tokenizer.getNextToken())) { // consume '(' and check ')'
            while (true) {
                if (auto arg = parseExpression())
                    args.push_back(std::move(arg));
                else
                    return nullptr;

                if (token::is_r_paren(tokenizer.curToken()))
                    break;

                if (!token::is_comma(tokenizer.curToken()))
                    return logErrorE("exptected ')' or ',' in argument list");

                tokenizer.getNextToken(); // consume ','
            }
        }

        tokenizer.getNextToken(); // consume ')'
        return std::make_unique<CallExprAST>(std::move(id_name), std::move(args));
    }

    // primary ::= identifier-expr | number-expr | paren-expr
    std::unique_ptr<ExprAST> parsePrimary() {
        if (token::is_identifier(tokenizer.curToken())) {
            return parseIdentifierExpr();
        } else if (token::is_num(tokenizer.curToken())) {
            return parseNumExpr();
        } else if (token::is_l_paren(tokenizer.curToken())) {
            return parsePrimary();
        } else {
            return logErrorE("unknown token when expecting an expression");
        }
    }

    // binop-rhs ::= (binop primary)*
    // binop = '+' | '-' | ...
    std::unique_ptr<ExprAST> parseBinOpRHS(int expr_prio, std::unique_ptr<ExprAST> lhs) {
        while (true) {
            auto binop = token::get_punc(tokenizer.curToken());
            if (binop.empty())
                return lhs;

            int tok_prio = getBinOpPrio(binop);
            if (tok_prio < expr_prio)
                return lhs;

            tokenizer.getNextToken(); // consume binop

            auto rhs = parsePrimary();
            if (!rhs)
                return nullptr;

            auto nextop = token::get_punc(tokenizer.curToken());
            int next_prio = getBinOpPrio(nextop);
            if (tok_prio < next_prio) {
                rhs = parseBinOpRHS(tok_prio + 1, std::move(rhs));
                if (!rhs)
                    return nullptr;
            }

            lhs = std::make_unique<BinaryExprAST>(std::move(binop), std::move(lhs), std::move(rhs));
        }
    }

    // expression ::= primary binop-rhs
    std::unique_ptr<ExprAST> parseExpression() {
        auto lhs = parsePrimary();
        if (!lhs)
            return nullptr;

        return parseBinOpRHS(0, std::move(lhs));
    }

    // prototype ::= identifier '(' (identifier (',' identifier)*)? ')'
    std::unique_ptr<PrototypeAST> parsePrototype() {
        if (!token::is_identifier(tokenizer.curToken())) {
            return logErrorP("expected function name in prototype");
        }

        std::string fn_name = token::get_identifier(tokenizer.curToken());
        if (!token::is_l_paren(tokenizer.getNextToken())) {
            return logErrorP("expected '(' in prototype. curTok: ", tokenizer.curToken());
        }

        std::vector<std::string> arg_names;
        while (true) {
            if (!token::is_identifier(tokenizer.getNextToken()))
                return logErrorP("expected identifier in prototype. curTok: ", tokenizer.curToken());
            arg_names.push_back(token::get_identifier(tokenizer.curToken()));

            tokenizer.getNextToken();
            if (token::is_comma(tokenizer.curToken())) {
                continue;
            } else if (token::is_r_paren(tokenizer.curToken())) {
                tokenizer.getNextToken(); // consume ')'
                break;
            } else {
                return logErrorP("expected ')' or ',' in prototype. curTok: ", tokenizer.curToken());
            }
        }

        return std::make_unique<PrototypeAST>(std::move(fn_name), std::move(arg_names));
    }

    // external ::= 'extern' prototype
    std::unique_ptr<PrototypeAST> parseExtern() {
        tokenizer.getNextToken(); // consume 'extern'
        return parsePrototype();
    }

    // definition ::= 'def' prototype expression
    std::unique_ptr<FunctionAST> parseDefinition() {
        tokenizer.getNextToken(); // consume 'def'

        auto proto = parsePrototype();
        if (!proto)
            return nullptr;

        auto body = parseExpression();
        if (!body)
            return nullptr;

        return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
    }

    // toplevelexpr ::= expression
    std::unique_ptr<FunctionAST> parseTopLevelExpr() {
        if (auto expr = parseExpression()) {
            auto proto = std::make_unique<PrototypeAST>("anonymous", std::vector<std::string>{});
            return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
        }
        return nullptr;
    }

    explicit ParserImpl(Tokenizer &&tok)
            : tokenizer{std::move(tok)} {}

    Tokenizer tokenizer;

    std::unordered_map<std::string, int> binop_prio;

    int getBinOpPrio(const std::string &op) {
        if (auto p = binop_prio.find(op); p != binop_prio.end()) {
            return p->second;
        }
        return -1;
    }
};

Parser::Parser(Tokenizer &&tok)
        : impl{std::make_shared<ParserImpl>(std::move(tok))} {
    impl->binop_prio["<"] = 10;
    impl->binop_prio[">"] = 10;
    impl->binop_prio["+"] = 20;
    impl->binop_prio["-"] = 20;
    impl->binop_prio["*"] = 40;
    impl->binop_prio["/"] = 40;
}

bool Parser::parse() {
    namespace hana = boost::hana;

    const auto &token = impl->tokenizer.getNextToken();

    if (token::is_eof(token)) {
        return false;
    } else if (token::is_semicolon(token)) {
        parse();
    } else if (token::is_def(token)) {
        auto def = impl->parseDefinition();
        if (def_handler && def)
            def_handler(*def);
    } else if (token::is_extern(token)) {
        auto proto = impl->parseExtern();
        if (extern_handler && proto)
            extern_handler(*proto);
    } else {
        auto toplevel = impl->parseTopLevelExpr();
        if (toplevel_handler && toplevel)
            toplevel_handler(*toplevel);
    }
    return true;
}
}