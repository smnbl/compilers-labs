#ifndef PARSER_HPP
#define PARSER_HPP

#include "ast/ast.hpp"
#include "lexer/token.hpp"

#include <stdexcept>
#include <string>
#include <vector>

class Parser {
  public:
    Parser(const std::vector<Token> &tokens);
    ast::Ptr<ast::Base> parse();
    bool hadError() const;

    struct ParserException : public std::runtime_error {
        Token token;

        ParserException(const Token &token, const std::string &message)
            : std::runtime_error(message), token(token) {}
    };

  private:
    // The list of tokens that was created by the lexer.
    std::vector<Token> tokens;

    // Iterator to the current token.
    std::vector<Token>::const_iterator current;

    // Flag that is set when an error occurs.
    bool errorFlag = false;

    // Returns true if the entire input is processed.
    bool isAtEnd() const;

    // Advances the parser by one token.
    void advance();

    // Peeks the next token in the input stream.
    Token peek() const;

    // Peeks two tokens forward in the input stream.
    Token peekNext() const;

    // Ensures that the next token is of the given type, returns that token, and
    // advances the parser.
    Token eat(TokenType expected, const std::string &errorMessage = "");

    // Reports an error at the current position.
    ParserException error(const std::string &message) const;

    // Parsing functions. Each of these functions corresponds (roughly speaking)
    // to a non-terminal symbol in the grammar.

    ast::Ptr<ast::Program> parseProgram();
    ast::Ptr<ast::FuncDecl> parseFuncDecl();
    ast::List<ast::Ptr<ast::VarDecl>> parseFuncDeclArgs();
    ast::Ptr<ast::Stmt> parseStmt();
    ast::Ptr<ast::Stmt> parseForInit();
    ast::Ptr<ast::CompoundStmt> parseCompoundStmt();

    ast::Ptr<ast::Expr> parseExpr();
    ast::Ptr<ast::Expr> parseAtom();
    ast::Ptr<ast::IntLiteral> parseIntLiteral();

    // ASSIGNMENT: Declare additional parsing functions here.

};

#endif /* end of include guard: PARSER_HPP */
