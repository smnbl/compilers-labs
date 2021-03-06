#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum class TokenType {
    // Keywords
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,

    // Identifiers
    IDENTIFIER,

    // Literals
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,

    // Operators
    EQUALS,              // =
    EQUALS_EQUALS,       // ==
    BANG_EQUALS,         // !=
    LESS_THAN,           // <
    LESS_THAN_EQUALS,    // <=
    GREATER_THAN,        // >
    GREATER_THAN_EQUALS, // >=
    PLUS,                // +
    MINUS,               // -
    STAR,                // *
    SLASH,               // /
    CARET,               // ^
    PERCENT,             // %

    // Punctuation
    LEFT_PAREN,    // (
    RIGHT_PAREN,   // )
    LEFT_BRACE,    // {
    RIGHT_BRACE,   // }
    LEFT_BRACKET,  // [
    RIGHT_BRACKET, // ]
    COMMA,         // ,
    SEMICOLON,     // ;
};

std::string token_type_to_string(TokenType type);

struct Location {
    Location() : line(0), col(0) {}
    Location(unsigned int line, unsigned int col) : line(line), col(col) {}
    unsigned int line;
    unsigned int col;
};

struct Token {
    Token(TokenType type, Location begin, Location end,
          const std::string &lexeme)
        : type(type), begin(begin), end(end), lexeme(lexeme) {}

    TokenType type;
    Location begin;
    Location end;
    std::string lexeme;
};

#endif /* end of include guard: TOKEN_HPP */
