#ifndef CODEGENEXCEPTION_HPP
#define CODEGENEXCEPTION_HPP

#include <stdexcept>

namespace codegen_x64 {
struct CodegenException : public std::runtime_error {
    CodegenException(const std::string &message)
        : std::runtime_error(message) {}
};
} // namespace codegen_x64

#endif /* end of include guard: CODEGENEXCEPTION_HPP */
