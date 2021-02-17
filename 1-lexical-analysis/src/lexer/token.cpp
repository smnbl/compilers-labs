#include "lexer/token.hpp"

#include <stdexcept>

std::string token_type_to_string(TokenType type) {
    switch (type) {
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::INT_LITERAL:
        return "INT_LITERAL";
    case TokenType::FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::EQUALS:
        return "EQUALS";
    case TokenType::EQUALS_EQUALS:
        return "EQUALS_EQUALS";
    case TokenType::BANG_EQUALS:
        return "BANG_EQUALS";
    case TokenType::LESS_THAN:
        return "LESS_THAN";
    case TokenType::LESS_THAN_EQUALS:
        return "LESS_THAN_EQUALS";
    case TokenType::GREATER_THAN:
        return "GREATER_THAN";
    case TokenType::GREATER_THAN_EQUALS:
        return "GREATER_THAN_EQUALS";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::CARET:
        return "CARET";
    case TokenType::PERCENT:
        return "PERCENT";
    case TokenType::LEFT_PAREN:
        return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TokenType::LEFT_BRACE:
        return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TokenType::LEFT_BRACKET:
        return "LEFT_BRACKET";
    case TokenType::RIGHT_BRACKET:
        return "RIGHT_BRACKET";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    default:
        throw std::invalid_argument("Unknown token type!");
    }
}
