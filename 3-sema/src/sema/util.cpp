#include "sema/semanticexception.hpp"
#include "sema/util.hpp"

#include <fmt/core.h>

llvm::Type *sema::Util::parseLLVMType(llvm::LLVMContext &ctx,
                                      const std::string &type) {
    if (type == "int")
        return llvm::Type::getInt64Ty(ctx);
    else if (type == "float")
        return llvm::Type::getFloatTy(ctx);
    else if (type == "string")
        return llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
    else if (type == "void")
        return llvm::Type::getVoidTy(ctx);
    else
        throw SemanticException(fmt::format("Unknown type '{}'", type));
}

llvm::Type *sema::Util::parseLLVMType(llvm::LLVMContext &ctx,
                                      const Token &type) {
    try {
        return parseLLVMType(ctx, type.lexeme);
    } catch (const SemanticException &e) {
        throw SemanticException(fmt::format("Unknown type '{}'", type.lexeme),
                                type.begin);
    }
}

std::string sema::Util::llvm_type_to_string(llvm::Type *type) {
    std::string ret;

    llvm::raw_string_ostream rso{ret};
    type->print(rso);

    return ret;
}
