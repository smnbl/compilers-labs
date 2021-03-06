#ifndef SEMANTICEXCEPTION_HPP
#define SEMANTICEXCEPTION_HPP

#include "lexer/token.hpp"

#include <stdexcept>
#include <string>

namespace sema {
struct SemanticException : public std::runtime_error {
    Location location;

    SemanticException(const std::string &message,
                      const Location &location = Location())
        : std::runtime_error(message), location(location) {}
};
} // namespace sema

#endif /* end of include guard: SEMANTICEXCEPTION_HPP */
