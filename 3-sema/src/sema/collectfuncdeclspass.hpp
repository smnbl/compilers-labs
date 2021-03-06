#ifndef COLLECTFUNCDECLSPASS_HPP
#define COLLECTFUNCDECLSPASS_HPP

#include "ast/ast.hpp"
#include "ast/visitor.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

#include <map>
#include <string>

namespace sema {
// AST pass that collects function declarations, and creates a table that maps
// the function name to the function type. This type contains the function's
// argument types, and return type. This pass also verifies that functions are
// not redefined.
class CollectFuncDeclsPass : public ast::Visitor<CollectFuncDeclsPass> {
  public:
    using FunctionTable = std::map<std::string, llvm::Type *>;

    CollectFuncDeclsPass(llvm::LLVMContext &ctx) : ctx(ctx) {}

    FunctionTable getFunctionTable() const { return function_table; }

    void visitFuncDecl(ast::FuncDecl &node);

  private:
    // Maps the name of a function to its LLVM type (containing return type and
    // argument types).
    FunctionTable function_table;

    llvm::LLVMContext &ctx;
};
} // namespace sema

#endif /* end of include guard: COLLECTFUNCDECLSPASS_HPP */
