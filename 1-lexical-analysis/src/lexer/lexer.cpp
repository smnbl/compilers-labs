#include "lexer.hpp"

#include "llvm/Support/Debug.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>
#include <fmt/core.h>
#include <iostream>
#include <iterator>

#define DEBUG_TYPE "lexer"

Lexer::Lexer(const std::string &input)
    : input(input), begin_location(1, 1), end_location(1, 1) {
    begin = end = std::begin(this->input);
}

std::vector<Token> Lexer::getTokens() {
    while (!isAtEnd()) {
        lexToken();

        begin = end;
        begin_location = end_location;
    }

    return tokens;
}

bool Lexer::hadError() const { return errorFlag; }

bool Lexer::isAtEnd() const { return end == std::end(input); }

void Lexer::lexToken() {
    // ASSIGNMENT: Implement the lexical analyser here.
    char c = peek();
    advance();

    switch (c) {
        case ' ':  
        case '\n':
        case '\r':
        case '\t':
            break;
        case ',':
            emitToken(TokenType::COMMA);
            break;
        case ';':
            emitToken(TokenType::SEMICOLON);
            break;
        case '(':
            emitToken(TokenType::LEFT_PAREN);
            break;
        case ')':
            emitToken(TokenType::RIGHT_PAREN);
            break;
        case '}':
            emitToken(TokenType::RIGHT_BRACE);
            break;
        case '{':
            emitToken(TokenType::LEFT_BRACE);
            break;
        case '[':
            emitToken(TokenType::LEFT_BRACKET);
            break;
        case ']':
            emitToken(TokenType::RIGHT_BRACKET);
            break;
        case '=':
            
            if (peek() == '='){
                advance();
                emitToken(TokenType::EQUALS_EQUALS);
            }
            else{
                emitToken(TokenType::EQUALS);
            }
            break;
        case '+':
            emitToken(TokenType::PLUS);
            break;
        case '-':
            emitToken(TokenType::MINUS);
            break;
        case '*':
            emitToken(TokenType::STAR);
            break;
        case '/':
            if (peek() == '/'){
                while(peek()!= '\n'){
                advance();
                }    
            }
            else{
                emitToken(TokenType::SLASH);
            }
            break;
        case '^':
            emitToken(TokenType::CARET);
            break;
        case '%':
            emitToken(TokenType::PERCENT);
            break;
        case '<':
            
            if (peek() == '='){
                advance();
                emitToken(TokenType::LESS_THAN_EQUALS);
            }
            else{
                emitToken(TokenType::LESS_THAN);
            }
            break;
        case '>':
            
            if (peek() == '='){
                advance();
                emitToken(TokenType::GREATER_THAN_EQUALS);
            }
            else{
                emitToken(TokenType::GREATER_THAN);
            }
            break;

        case '!':
            if (peek() == '='){
                advance();
                emitToken(TokenType::BANG_EQUALS);
                
            }
            else{
                error(fmt::format("Expected '=' after '!'"));
            }
            break;
        case '.': //float starts with .
            while (isdigit(peek()))
            {
                advance();
            }
            if (peek() =='.'){
                advance();
                error(fmt::format("Float literals must only contain one decimal point"));
            }
            else{
                emitToken(TokenType::FLOAT_LITERAL);
            }
            break;

        case '"':
            while (peek() != '"'){
                if (peek()  == '\n'){
                    error(fmt::format("Unterminated string literal"));
                    break;
                }
                advance();
            }
            advance();
            emitToken(TokenType::STRING_LITERAL);
            break;
            

        default:
            if (isdigit(c)){
                while(isdigit(peek())){
                    advance();
                }
                if (peek() == '.'){ //float which starts with number
                    advance();
                    while(isdigit(peek())){
                    advance();
                    }
                    if (peek() == '.'){
                        advance();
                        error(fmt::format("Float literals must only contain one decimal point"));
                    }
                    else{
                        emitToken(TokenType::FLOAT_LITERAL);
                    }

                }
                else{
                    emitToken(TokenType::INT_LITERAL);
                }
                
            }
            else if (isalpha(c)){ //first is a letter
                switch (c)
                {
                case 'i':
                    
                    if (peek() == 'f'){
                        advance();
                        if (!(isalpha(peek()) or isdigit(peek()) or peek() == '_')){
                            emitToken(TokenType::IF);
                            break;
                        }
                        
                    while (isalpha(peek()) or isdigit(peek()) or peek() == '_'){
                        advance();
                    }
                    emitToken(TokenType::IDENTIFIER);
                     }
                    break;
                case 'f':
                    if (peek() == 'o'){
                        advance();
                        if (peek() == 'r'){
                            advance();
                            if (!(isalpha(peek()) or isdigit(peek()) or peek() == '_')){
                                emitToken(TokenType::FOR);
                                break;
                            }
                            
                            
                        }
                    }
                    while (isalpha(peek()) or isdigit(peek()) or peek() == '_'){
                        advance();
                    }
                    emitToken(TokenType::IDENTIFIER);
                
                    break;
                case 'e':
                    if (peek() == 'l'){
                        advance();
                        if (peek() == 's'){
                            advance();
                            if (peek() == 'e'){
                                advance();
                                if (!(isalpha(peek()) or isdigit(peek()) or peek() == '_')){
                                    emitToken(TokenType::ELSE);
                                    break;
                                }
                                
                            }
                            
                        }
                    }
                    while (isalpha(peek()) or isdigit(peek()) or peek() == '_'){
                        advance();
                    }
                    emitToken(TokenType::IDENTIFIER);
                
                    break;
                case 'w':
                    if (peek() == 'h'){
                        advance();
                        if (peek() == 'i'){
                            advance();
                            if (peek() == 'l'){
                                advance();
                                if (peek()=='e'){
                                    advance();
                                    if (!(isalpha(peek()) or isdigit(peek()) or peek() == '_')){
                                        emitToken(TokenType::WHILE);
                                        break;
                                    }
                                
                                }
                            }
                            
                        }
                    }
                    while (isalpha(peek()) or isdigit(peek()) or peek() == '_'){
                        advance();
                    }
                    emitToken(TokenType::IDENTIFIER);
                
                    break;

                case 'r':
                    if (peek() == 'e'){
                        advance();
                        if (peek() == 't'){
                            advance();
                            if (peek() == 'u'){
                                advance();
                                if (peek()=='r'){
                                    advance();
                                    if (peek() == 'n'){
                                        advance();
                                        if (!(isalpha(peek()) or isdigit(peek()) or peek() == '_')){
                                            emitToken(TokenType::RETURN);
                                            break;
                                        }
                                    }
                                
                                }
                            }
                            
                        }
                    }
                    while (isalpha(peek()) or isdigit(peek()) or peek() == '_'){
                        advance();
                    }
                    emitToken(TokenType::IDENTIFIER);
                
                    break;
                default:
                    break;
                } 
            }
            else{
                error(fmt::format("Invalid character '{}'", c));
            }
            break;
    }
}

void Lexer::emitToken(TokenType type) {
    tokens.emplace_back(type, begin_location, end_location, getLexeme());
}

void Lexer::advance() {
    // Update location information
    if (peek() == '\n') {
        ++end_location.line;
        end_location.col = 1;
    } else {
        ++end_location.col;
    }

    if (!isAtEnd())
        ++end;
}

char Lexer::peek() {
    if (!isAtEnd())
        return *end;
    else
        return '\0';
}

void Lexer::error(const std::string &message) {
    errorFlag = true;
    llvm::WithColor::error(llvm::errs(), "lexer") << fmt::format(
        "{}:{}: {}\n", end_location.line, end_location.col, message);
}

std::string Lexer::getLexeme() const { return std::string{begin, end}; }
