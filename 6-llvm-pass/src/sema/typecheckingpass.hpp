#ifndef TYPECHECKINGPASS_HPP
#define TYPECHECKINGPASS_HPP

#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "sema/collectfuncdeclspass.hpp"
#include "sema/scoperesolutionpass.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

#include <map>

namespace sema {
// AST pass that determines the type of each subexpression used in the input
// program, and that verifies the typing rules of micro-C.
class TypeCheckingPass : public ast::Visitor<TypeCheckingPass, llvm::Type *> {
  public:
    TypeCheckingPass(llvm::LLVMContext &ctx);

    void setFunctionTable(
        const CollectFuncDeclsPass::FunctionTable &function_table) {
        this->function_table = function_table;
    }

    void setSymbolTable(const ScopeResolutionPass::SymbolTable &symbol_table) {
        this->symbol_table = symbol_table;
    }

    using TypeTable = std::map<ast::Base *, llvm::Type *>;

    TypeTable getTypeTable() const { return type_table; }

    llvm::Type *visitFuncDecl(ast::FuncDecl &node);
    llvm::Type *visitIfStmt(ast::IfStmt &node);
    llvm::Type *visitWhileStmt(ast::WhileStmt &node);
    llvm::Type *visitReturnStmt(ast::ReturnStmt &node);
    llvm::Type *visitVarDecl(ast::VarDecl &node);
    llvm::Type *visitArrayDecl(ast::ArrayDecl &node);
    llvm::Type *visitBinaryOpExpr(ast::BinaryOpExpr &node);
    llvm::Type *visitUnaryOpExpr(ast::UnaryOpExpr &node);
    llvm::Type *visitIntLiteral(ast::IntLiteral &node);
    llvm::Type *visitFloatLiteral(ast::FloatLiteral &node);
    llvm::Type *visitStringLiteral(ast::StringLiteral &node);
    llvm::Type *visitVarRefExpr(ast::VarRefExpr &node);
    llvm::Type *visitArrayRefExpr(ast::ArrayRefExpr &node);
    llvm::Type *visitFuncCallExpr(ast::FuncCallExpr &node);

  private:
    // Maps each variable and array to its LLVM type.
    TypeTable type_table;

    // Maps the name of a function to its LLVM type (containing the return type
    // and argument types).
    CollectFuncDeclsPass::FunctionTable function_table;

    // Maps the use of a variable to its declaration.
    ScopeResolutionPass::SymbolTable symbol_table;

    llvm::LLVMContext &ctx;

    // Remember the current function we are in.
    ast::FuncDecl *current_function;

    llvm::Type *T_void;   // LLVM's void type
    llvm::Type *T_int;    // LLVM's i64 type
    llvm::Type *T_float;  // LLVM's float type
    llvm::Type *T_string; // LLVM's i8* type
};
} // namespace sema

#endif /* end of include guard: TYPECHECKINGPASS_HPP */
