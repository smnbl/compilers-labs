#include "sema/collectfuncdeclspass.hpp"
#include "sema/semanticexception.hpp"
#include "sema/util.hpp"

#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Debug.h"

#include <fmt/core.h>
#include <vector>

#define DEBUG_TYPE "collectfuncdeclspass"

using namespace ast;

void sema::CollectFuncDeclsPass::visitFuncDecl(FuncDecl &node) {
    LLVM_DEBUG(llvm::dbgs()
               << "Checking FuncDecl: " << node.name.lexeme << "\n");

    // Check for redefinitions.
    const std::string name = node.name.lexeme;
    if (function_table.find(name) != std::end(function_table)) {
        throw SemanticException(
            fmt::format("Cannot redefine function '{}'", name),
            node.name.begin);
    }

    // Create LLVM type of function.
    std::vector<llvm::Type *> argTypes;

    for (const Ptr<VarDecl> arg : node.arguments) {
        argTypes.emplace_back(sema::Util::parseLLVMType(ctx, arg->type));
    }

    llvm::Type *retTy = sema::Util::parseLLVMType(ctx, node.returnType);

    llvm::Type *funcType = llvm::FunctionType::get(retTy, argTypes, false);

    function_table[name] = funcType;
}
