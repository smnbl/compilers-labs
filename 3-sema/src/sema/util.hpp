#ifndef SEMA_UTIL_HPP
#define SEMA_UTIL_HPP

#include "lexer/token.hpp"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"

#include <string>

namespace sema {
struct Util {
    static llvm::Type *parseLLVMType(llvm::LLVMContext &ctx,
                                     const std::string &type);
    static llvm::Type *parseLLVMType(llvm::LLVMContext &ctx, const Token &type);
    static std::string llvm_type_to_string(llvm::Type *type);
};
} // namespace sema

#endif /* end of include guard: SEMA_UTIL_HPP */
