#ifndef CODEGEN_LLVM_HPP
#define CODEGEN_LLVM_HPP

#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "lexer/token.hpp"
#include "sema/collectfuncdeclspass.hpp"
#include "sema/scoperesolutionpass.hpp"
#include "sema/typecheckingpass.hpp"

#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

#include <map>
#include <memory>
#include <stack>

namespace codegen_llvm {
class CodeGeneratorLLVM
    : public ast::Visitor<CodeGeneratorLLVM, llvm::Value *> {
  public:
    CodeGeneratorLLVM(
        llvm::LLVMContext &ctx, const std::string &filepath,
        const sema::CollectFuncDeclsPass::FunctionTable &function_table,
        const sema::ScopeResolutionPass::SymbolTable &symbol_table,
        const sema::TypeCheckingPass::TypeTable &type_table);
    llvm::Module &getModule() const { return *module; }

    llvm::Value *visitFuncDecl(ast::FuncDecl &node);
    llvm::Value *visitIfStmt(ast::IfStmt &node);
    llvm::Value *visitWhileStmt(ast::WhileStmt &node);
    llvm::Value *visitReturnStmt(ast::ReturnStmt &node);
    llvm::Value *visitVarDecl(ast::VarDecl &node);
    llvm::Value *visitArrayDecl(ast::ArrayDecl &node);
    llvm::Value *visitBinaryOpExpr(ast::BinaryOpExpr &node);
    llvm::Value *visitUnaryOpExpr(ast::UnaryOpExpr &node);
    llvm::Value *visitIntLiteral(ast::IntLiteral &node);
    llvm::Value *visitFloatLiteral(ast::FloatLiteral &node);
    llvm::Value *visitStringLiteral(ast::StringLiteral &node);
    llvm::Value *visitVarRefExpr(ast::VarRefExpr &node);
    llvm::Value *visitArrayRefExpr(ast::ArrayRefExpr &node);
    llvm::Value *visitFuncCallExpr(ast::FuncCallExpr &node);

    // Finalize codegen. This must be called before the module is dumped.
    void finalize();

  private:
    // LLVM context.
    llvm::LLVMContext &context;

    llvm::Type *T_void;   // LLVM's void type
    llvm::Type *T_int;    // LLVM's i64 type
    llvm::Type *T_float;  // LLVM's float type
    llvm::Type *T_string; // LLVM's i8* type

    // LLVM IR Builder.
    llvm::IRBuilder<> builder;

    // LLVM Module containing the emitted LLVM IR.
    std::unique_ptr<llvm::Module> module;

    // Debug information builder.
    llvm::DIBuilder debug_builder;

    // Debug information compile unit.
    llvm::DICompileUnit *debug_compile_unit;

    // Debug information scope stack.
    std::stack<llvm::DIScope *> debug_scopes;

    // Function table
    sema::CollectFuncDeclsPass::FunctionTable function_table;

    // Symbol table.
    sema::ScopeResolutionPass::SymbolTable symbol_table;

    // Type table.
    sema::TypeCheckingPass::TypeTable type_table;

    // Create an alloca in the entry block of the current function.
    llvm::AllocaInst *
    createAllocaInEntryBlock(llvm::Type *type,
                             llvm::Value *array_size = nullptr);

    // Returns true if the current basic block is terminated, and false
    // otherwise.
    bool isCurrentBasicBlockTerminated();

    // Maps AST nodes of variable/array declarations to their alloca.
    std::map<ast::Base *, llvm::AllocaInst *> alloca_table;

    // Finds the alloca instruction corresponding to the given node.
    // This 'node' must be a VarRefExpr or an ArrayRefExpr.
    llvm::AllocaInst *findAllocaForRef(ast::Base &node);

    // Emit a debug information location.
    void emitDebugLocation(const Location &loc);

    // Clear the debug information location.
    void clearDebugLocation();
};
} // namespace codegen_llvm

#endif /* end of include guard: CODEGEN_LLVM_HPP */
