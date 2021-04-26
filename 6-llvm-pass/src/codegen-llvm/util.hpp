#ifndef CODEGEN_LLVM_UTIL_HPP
#define CODEGEN_LLVM_UTIL_HPP

#include "lexer/token.hpp"

#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Type.h"

namespace codegen_llvm {
struct Util {
    static llvm::DIType *getDIType(llvm::DIBuilder &dibuilder,
                                   const std::string &type);
    static llvm::DIType *getDIType(llvm::DIBuilder &dibuilder, const Token &type);
};
} // namespace codegen_llvm

#endif /* end of include guard: CODEGEN_LLVM_UTIL_HPP */
