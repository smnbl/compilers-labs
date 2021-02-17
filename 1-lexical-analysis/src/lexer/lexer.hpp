#ifndef LEXER_HPP
#define LEXER_HPP

#include "lexer/token.hpp"

#include <string>
#include <vector>

class Lexer {
  public:
    Lexer(const std::string &input);
    std::vector<Token> getTokens();
    bool hadError() const;

  private:
    // String containing the input.
    std::string input;

    // Iterators to the beginning and one-past-the-end of the current token.
    std::string::const_iterator begin, end;

    // Location of begin and end in the source file.
    Location begin_location, end_location;

    // List of tokens
    std::vector<Token> tokens;

    // Flag that is set when an error occurs.
    bool errorFlag = false;

    // Returns true if the entire input is processed.
    bool isAtEnd() const;

    // Lexes the next token in the input.
    void lexToken();

    // Helper method to add a token to the list of tokens.
    void emitToken(TokenType type);

    // Adds the next character in the input to the current token.
    void advance();

    // Peeks the next character in the input stream, without adding it to the
    // current token.
    char peek();

    // Reports an error at the current position.
    void error(const std::string &message);

    // Get the current lexeme.
    std::string getLexeme() const;

    // ASSIGNMENT: Declare any helper function you use here (if any).
};

#endif /* end of include guard: LEXER_HPP */
