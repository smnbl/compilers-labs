#include "parser/parser.hpp"

#include "llvm/Support/Debug.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

#include <fmt/core.h>
#include <iterator>

using namespace ast;
using std::make_shared;

#define DEBUG_TYPE "parser"

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {
    current = std::begin(this->tokens);
}

Ptr<Base> Parser::parse() {
    try {
        return parseProgram();
    }

    catch (ParserException &e) {
        errorFlag = true;
        llvm::WithColor::error(llvm::errs(), "parser") << fmt::format(
            "{}:{}: {}\n", e.token.begin.line, e.token.begin.col, e.what());
        return nullptr;
    }
}

bool Parser::hadError() const { return errorFlag; }

bool Parser::isAtEnd() const { return current == std::end(tokens); }

void Parser::advance() {
    if (!isAtEnd())
        ++current;
}

Token Parser::peek() const {
    if (!isAtEnd())
        return *current;
    else
        throw error("Cannot peak beyond end-of-file!");
}

Token Parser::peekNext() const {
    auto it = current;

    if (it == std::end(tokens))
        throw error("Cannot peak beyond end-of-file!");

    std::advance(it, 1);

    if (it == std::end(tokens))
        throw error("Cannot peak beyond end-of-file!");

    return *it;
}

Parser::ParserException Parser::error(const std::string &message) const {
    // NOTE: For now, we stop the parser completely when we encounter an error.
    // A better solution would be to implement some form of error recovery (e.g.
    // skipping to tokens in the follow set, synchronisation on statement
    // boundaries, ...)
    return ParserException(*current, message);
}

Token Parser::eat(TokenType expected, const std::string &errorMessage) {
    Token nextToken = peek();
    TokenType actual = nextToken.type;

    if (expected == actual) {
        advance();
        return nextToken;
    }

    if (!errorMessage.empty())
        throw error(errorMessage);
    else
        throw error(fmt::format("Expected token type '{}', but got '{}'",
                                token_type_to_string(expected),
                                token_type_to_string(actual)));
}

// program = function_decl*
Ptr<Program> Parser::parseProgram() {
    LLVM_DEBUG(llvm::dbgs() << "In parseProgram()\n");

    List<Ptr<FuncDecl>> decls;

    while (!isAtEnd()) {
        decls.push_back(parseFuncDecl());
    }

    return make_shared<Program>(decls);
}

// function_decl = IDENTIFIER IDENTIFIER "(" function_decl_args? ")" "{" stmt*
// "}"
Ptr<FuncDecl> Parser::parseFuncDecl() {
    LLVM_DEBUG(llvm::dbgs() << "In parseFuncDecl()\n");

    Token returnType = eat(TokenType::IDENTIFIER);
    Token name = eat(TokenType::IDENTIFIER);

    eat(TokenType::LEFT_PAREN);

    // Parse arguments
    List<Ptr<VarDecl>> arguments;

    if (peek().type != TokenType::RIGHT_PAREN) {
        arguments = parseFuncDeclArgs();
    }

    eat(TokenType::RIGHT_PAREN);
    Ptr<CompoundStmt> body = parseCompoundStmt();

    return make_shared<FuncDecl>(returnType, name, arguments, body);
}

// function_decl_args = IDENTIFIER IDENTIFIER ("," IDENTIFIER IDENTIFIER)*
List<Ptr<VarDecl>> Parser::parseFuncDeclArgs() {
    LLVM_DEBUG(llvm::dbgs() << "In parseFuncDeclArgs()\n");

    List<Ptr<VarDecl>> args;

    // Add first argument
    Token type = eat(TokenType::IDENTIFIER);
    Token name = eat(TokenType::IDENTIFIER);

    args.emplace_back(make_shared<VarDecl>(type, name));

    // Parse any remaining arguments
    while (peek().type == TokenType::COMMA) {
        eat(TokenType::COMMA);

        Token type = eat(TokenType::IDENTIFIER);
        Token name = eat(TokenType::IDENTIFIER);

        args.emplace_back(make_shared<VarDecl>(type, name));
    }

    return args;
}

// stmt = 
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" forinit expr ";" expr ")" stmt
//      | "while" "(" expr ")" stmt
//      | "return" expr? ";"
//      | exprstmt | vardeclstmt | arrdeclstmt | "{" stmt* "}" | ";"
// exprstmt = expr ";"
// vardeclstmt = IDENTIFIER IDENTIFIER ("=" expr)? ";"
// arrdeclstmt = IDENTIFIER IDENTIFIER "[" INTEGER "]" ";"
Ptr<Stmt> Parser::parseStmt() {
    LLVM_DEBUG(llvm::dbgs() << "In parseStmt()\n");

    // solution to 3.2: fixes the conflict (our grammar cannot be predicted based solely on the next token)
    // we have a VarRef | ArrayRef instead of a declaration
    if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::IDENTIFIER) {
        // vardeclstmt or arrdeclstmt 

        Token type = eat(TokenType::IDENTIFIER);

        Token name = eat(TokenType::IDENTIFIER);

        if (peek().type == TokenType::LEFT_BRACKET) {
            // arrdeclstmt
            eat(TokenType::LEFT_BRACKET);
            Ptr<IntLiteral> size = parseIntLiteral();
            eat(TokenType::RIGHT_BRACKET);
            eat(TokenType::SEMICOLON);

            return make_shared<ArrayDecl>(type, name, size);
        } else {
            // vardeclstmt
            Ptr<Expr> init = nullptr;

            if (peek().type == TokenType::EQUALS) {
                eat(TokenType::EQUALS);
                init = parseExpr();
            }

            eat(TokenType::SEMICOLON);

            return make_shared<VarDecl>(type, name, init);
        }
    }

    if (peek().type == TokenType::LEFT_BRACE) {
        // compoundstmt
        return parseCompoundStmt();
    }

    if (peek().type == TokenType::SEMICOLON) {
        // empty statement
        eat(TokenType::SEMICOLON);
        return make_shared<EmptyStmt>();
    }

    if (peek().type == TokenType::FOR) {
        // for statement
        // NOTE: We desugar for loops to while AST nodes, so that we do not need
        // to handle fors separately in the later phases.
        eat(TokenType::FOR);
        eat(TokenType::LEFT_PAREN);

        Ptr<Stmt> init = parseForInit();
        Ptr<Expr> condition = parseExpr();

        eat(TokenType::SEMICOLON);

        Ptr<Expr> increment = parseExpr();

        eat(TokenType::RIGHT_PAREN);

        Ptr<Stmt> body = parseStmt();

        // Transform for(init; cond; inc) body
        // to:
        // {
        //      init;
        //      while(cond) {
        //          {
        //              body
        //          }
        //          inc;
        //      }
        // }

        List<Ptr<Stmt>> bodyCompoundStmts{body};
        Ptr<Stmt> bodyCompound = make_shared<CompoundStmt>(bodyCompoundStmts);

        Ptr<Stmt> incrementStmt = make_shared<ExprStmt>(increment);
        List<Ptr<Stmt>> whileBodyStmts{bodyCompound, incrementStmt};
        Ptr<Stmt> whileBody = make_shared<CompoundStmt>(whileBodyStmts);

        Ptr<Stmt> whileStmt = make_shared<WhileStmt>(condition, whileBody);

        List<Ptr<Stmt>> outerBlockStmts{init, whileStmt};
        Ptr<Stmt> outerBlock = make_shared<CompoundStmt>(outerBlockStmts);

        return outerBlock;
    }

    // ASSIGNMENT: Add additional statements here
    if (peek().type == TokenType::IF) {
        eat(TokenType::IF);
        eat(TokenType::LEFT_PAREN);
        auto condition = parseExpr();
        eat(TokenType::RIGHT_PAREN);
        
        Ptr<Stmt> if_clause = parseStmt();
        Ptr<Stmt> else_clause = nullptr;

        if(peek().type == TokenType::ELSE) {
            eat(TokenType::ELSE);
            else_clause = parseStmt();
        }

        return make_shared<IfStmt>(condition, if_clause, else_clause);
    }

    if(peek().type == TokenType::WHILE) {
        eat(TokenType::WHILE);
        eat(TokenType::LEFT_PAREN);
        auto condition = parseExpr();
        eat(TokenType::RIGHT_PAREN);
        auto body = parseStmt();

        return make_shared<WhileStmt>(condition, body);
    }

    if(peek().type == TokenType::RETURN) {
        eat(TokenType::RETURN);
        if (peek().type != TokenType::SEMICOLON) {
            auto expr = parseExpr();
            eat(TokenType::SEMICOLON);
            return make_shared<ReturnStmt>(expr);
        }
        eat(TokenType::SEMICOLON);
        return make_shared<ReturnStmt>();
    }

    // exprstmt
    Ptr<Expr> expr = parseExpr();
    eat(TokenType::SEMICOLON);

    return make_shared<ExprStmt>(expr);
}

// forinit = exprstmt | vardeclstmt | ";"
Ptr<Stmt> Parser::parseForInit() {
    LLVM_DEBUG(llvm::dbgs() << "In parseForInit()\n");

    if (peek().type == TokenType::IDENTIFIER &&
        peekNext().type == TokenType::IDENTIFIER) {
        // vardeclstmt
        Token type = eat(TokenType::IDENTIFIER);
        Token name = eat(TokenType::IDENTIFIER);

        Ptr<Expr> init = nullptr;

        if (peek().type == TokenType::EQUALS) {
            eat(TokenType::EQUALS);
            init = parseExpr();
        }

        eat(TokenType::SEMICOLON);

        return make_shared<VarDecl>(type, name, init);
    }

    if (peek().type == TokenType::SEMICOLON) {
        // empty statement
        eat(TokenType::SEMICOLON);
        return make_shared<EmptyStmt>();
    }

    // exprstmt
    Ptr<Expr> expr = parseExpr();
    eat(TokenType::SEMICOLON);

    return make_shared<ExprStmt>(expr);
}

// compoundstmt = "{" stmt* "}"
Ptr<CompoundStmt> Parser::parseCompoundStmt() {
    LLVM_DEBUG(llvm::dbgs() << "In parseCompoundStmt()\n");

    List<Ptr<Stmt>> body;
    eat(TokenType::LEFT_BRACE);

    while (peek().type != TokenType::RIGHT_BRACE) {
        auto stmt = parseStmt();
        if (stmt)
            body.emplace_back(stmt);
    }

    eat(TokenType::RIGHT_BRACE);
    return make_shared<CompoundStmt>(body);
}

// expr = atom
Ptr<Expr> Parser::parseExpr() {
    LLVM_DEBUG(llvm::dbgs() << "In parseExpr()\n");

    return parseAssignment();
}

Ptr<Expr> Parser::parseAssignment() {
    auto lhs = parseEquality();

    if (peek().type != TokenType::EQUALS) {
       return lhs; 
    }

    auto tok = eat(TokenType::EQUALS);
    auto rhs = parseAssignment();

    return make_shared<BinaryOpExpr>(lhs, tok, rhs);
}

Ptr<Expr> Parser::parseEquality() {
    auto lhs = parseComparison();

    if (peek().type == TokenType::EQUALS_EQUALS || peek().type == TokenType::BANG_EQUALS) {
        auto tok = eat(peek().type);
        auto rhs = parseComparison();
        if (peek().type == TokenType::EQUALS_EQUALS || peek().type == TokenType::BANG_EQUALS) {
            throw error("non-associative operators may not be used multiple times in a row");
        }
        return make_shared<BinaryOpExpr>(lhs, tok, rhs);
    }

    return lhs;
}

Ptr<Expr> Parser::parseComparison() {
    auto lhs = parseAdditive();

    if (   peek().type == TokenType::LESS_THAN 
        || peek().type == TokenType::LESS_THAN_EQUALS 
        || peek().type == TokenType::GREATER_THAN
        || peek().type == TokenType::GREATER_THAN_EQUALS) {

        auto tok = eat(peek().type);
        auto rhs = parseAdditive();

        if (   peek().type == TokenType::LESS_THAN 
            || peek().type == TokenType::LESS_THAN_EQUALS 
            || peek().type == TokenType::GREATER_THAN
            || peek().type == TokenType::GREATER_THAN_EQUALS) {
            throw error("non-associative operators may not be used multiple times in a row");
        }

        return make_shared<BinaryOpExpr>(lhs, tok, rhs);
    }

    return lhs;
}

Ptr<Expr> Parser::parseAdditive() {
    Ptr<Expr> expr = parseMultiplicative();

    // perform left-to-right associativity using a while loop
    while (1) {
        if(peek().type == TokenType::PLUS) {
            auto tok = eat(TokenType::PLUS);
            auto c2 = parseMultiplicative();
            expr = make_shared<BinaryOpExpr>(expr, tok, c2);
        } else if(peek().type == TokenType::MINUS) {
            auto tok = eat(TokenType::MINUS);
            auto c2 = parseMultiplicative();
            expr = make_shared<BinaryOpExpr>(expr, tok, c2);
        } else {
            break;
        }
    }

    return expr;
}

Ptr<Expr> Parser::parseMultiplicative() {
    Ptr<Expr> expr = parseUnary();

    // perform left-to-right associativity using a while loop
    while (1) {
        if(peek().type == TokenType::STAR) {
            auto tok = eat(TokenType::STAR);
            auto c2 = parseUnary();
            expr = make_shared<BinaryOpExpr>(expr, tok, c2);
        } else if(peek().type == TokenType::SLASH) {
            auto tok = eat(TokenType::SLASH);
            auto c2 = parseUnary();
            expr = make_shared<BinaryOpExpr>(expr, tok, c2);
        } else if(peek().type == TokenType::PERCENT) {
            auto tok = eat(TokenType::PERCENT);
            auto c2 = parseUnary();
            expr = make_shared<BinaryOpExpr>(expr, tok, c2);
        } else {
            break;
        }
    }

    return expr;
}

Ptr<Expr> Parser::parseUnary() {
    switch(peek().type) {
        case TokenType::MINUS: {
                auto tok = eat(TokenType::MINUS);
                auto value = parseUnary();
                return make_shared<UnaryOpExpr>(tok, value);
            }
        case TokenType::PLUS: {
                auto tok = eat(TokenType::PLUS);
                auto value = parseUnary();
                return make_shared<UnaryOpExpr>(tok, value);
            }
        default:
            return parseExponent();
    }
}

Ptr<Expr> Parser::parseExponent() {
    auto lhs = parseAtom();
    if (peek().type == TokenType::CARET) {
        auto tok = eat(TokenType::CARET);
        auto rhs = parseExponent();
        return make_shared<BinaryOpExpr>(lhs, tok, rhs);
    }

    return lhs;
}

// atom = INTEGER | '(' expr ')'
//        | FLOAT
//        | STRING
//        | IDENTIFIER ("[" expr "]")? // variable references
//        | IDENTIFIER "(" expr_arg_list? ")" // funccallexpression
Ptr<Expr> Parser::parseAtom() {
    LLVM_DEBUG(llvm::dbgs() << "In parseAtom()\n");

    if (peek().type == TokenType::INT_LITERAL) {
        // integer literal
        return parseIntLiteral();
    }

    if (peek().type == TokenType::LEFT_PAREN) {
        // parenthesised expression
        eat(TokenType::LEFT_PAREN);
        Ptr<Expr> expr = parseExpr();
        eat(TokenType::RIGHT_PAREN);

        return expr;
    }

    // ASSIGNMENT: Add additional atoms here.

    if (peek().type == TokenType::FLOAT_LITERAL) {
        return parseFloatLiteral();
    }

    if (peek().type == TokenType::STRING_LITERAL) {
        return parseStringLiteral();
    }

    if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::LEFT_PAREN) {
        return parseFuncCallExpr();
    }

    if (peek().type == TokenType::IDENTIFIER) {

        return parseRef();
    }

    throw error(fmt::format("Unexpected token type '{}' for atom",
                            token_type_to_string(peek().type)));
}

// ASSIGNMENT: Define additional parsing functions here.

// intliteral = INTEGER
Ptr<IntLiteral> Parser::parseIntLiteral() {
    LLVM_DEBUG(llvm::dbgs() << "In parseIntLiteral()\n");

    Token tok = eat(TokenType::INT_LITERAL);
    int value = std::stoi(tok.lexeme);

    return make_shared<IntLiteral>(value);
}

Ptr<FloatLiteral> Parser::parseFloatLiteral() {
    LLVM_DEBUG(llvm::dbgs() << "In parseFloatLiteral()\n");

    Token tok = eat(TokenType::FLOAT_LITERAL);
    float value = std::stof(tok.lexeme, nullptr);

    return make_shared<FloatLiteral>(value);
}

Ptr<StringLiteral> Parser::parseStringLiteral() {
    LLVM_DEBUG(llvm::dbgs() << "In parseStringLiteral()\n");

    Token tok = eat(TokenType::STRING_LITERAL);
    std::string value = tok.lexeme.substr(1, tok.lexeme.length() - 2);

    return make_shared<StringLiteral>(value);
}

// refexpr = | IDENTIFIER
//           | IDENTIFIER "[" expr "]"
Ptr<Expr> Parser::parseRef() {
    LLVM_DEBUG(llvm::dbgs() << "In parseRef()\n");

    Token tok = eat(TokenType::IDENTIFIER);

    if (peek().type == TokenType::LEFT_BRACKET) {
        eat(TokenType::LEFT_BRACKET);
        Ptr<Expr> expr = Parser::parseExpr();
        eat(TokenType::RIGHT_BRACKET);
        return make_shared<ArrayRefExpr>(tok, expr);
    }

    return make_shared<VarRefExpr>(tok);
}

Ptr<FuncCallExpr> Parser::parseFuncCallExpr() {
    List<Ptr<Expr>> arguments{};
    LLVM_DEBUG(llvm::dbgs() << "In parseFuncCall()\n");

    Token name = eat(TokenType::IDENTIFIER);
    eat(TokenType::LEFT_PAREN);
    
    if(peek().type != TokenType::RIGHT_PAREN) {
        // parse first argument
        arguments.push_back(parseExpr());

        // parse the rest
        while(peek().type == TokenType::COMMA) {
            eat(TokenType::COMMA);
            arguments.push_back(parseExpr());
        }
    }

    eat(TokenType::RIGHT_PAREN);

    return make_shared<FuncCallExpr>(name, arguments);
}
