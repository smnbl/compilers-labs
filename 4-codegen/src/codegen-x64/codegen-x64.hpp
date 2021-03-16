#ifndef CODEGEN_X64_HPP
#define CODEGEN_X64_HPP

#include "ast/ast.hpp"
#include "ast/visitor.hpp"
#include "codegen-x64/module.hpp"
#include "sema/scoperesolutionpass.hpp"

#include <array>
#include <map>
#include <string>

namespace codegen_x64 {
class CodeGeneratorX64 : public ast::Visitor<CodeGeneratorX64> {
  public:
    CodeGeneratorX64(const sema::ScopeResolutionPass::SymbolTable &symbol_table)
        : symbol_table(symbol_table) {}

    Module getModule() const { return module; }

    void visitFuncDecl(ast::FuncDecl &node);
    void visitIfStmt(ast::IfStmt &node);
    void visitWhileStmt(ast::WhileStmt &node);
    void visitReturnStmt(ast::ReturnStmt &node);
    void visitExprStmt(ast::ExprStmt &node);
    void visitVarDecl(ast::VarDecl &node);
    void visitArrayDecl(ast::ArrayDecl &node);
    void visitBinaryOpExpr(ast::BinaryOpExpr &node);
    void visitUnaryOpExpr(ast::UnaryOpExpr &node);
    void visitIntLiteral(ast::IntLiteral &node);
    void visitFloatLiteral(ast::FloatLiteral &node);
    void visitStringLiteral(ast::StringLiteral &node);
    void visitVarRefExpr(ast::VarRefExpr &node);
    void visitArrayRefExpr(ast::ArrayRefExpr &node);
    void visitFuncCallExpr(ast::FuncCallExpr &node);

  private:
    // Module containing the emitted assembly instructions.
    Module module;

    // Symbol table.
    sema::ScopeResolutionPass::SymbolTable symbol_table;

    // The label of the basic block corresponding to the current function's
    // exit.
    std::string function_exit;

    // System-V ABI definitions

    // Return register.
    const std::string abi_return_reg{"%rax"};

    // Registers used for parameters.
    const std::array<std::string, 6> abi_param_regs{"%rdi", "%rsi", "%rdx",
                                                    "%rcx", "%r8",  "%r9"};

    // Callee-saved registers.
    const std::array<std::string, 6> abi_callee_saved_regs{
        "%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15"};

    // Label counter
    unsigned int label_counter = 0;

    // Generate a new, unique label.
    std::string label(const std::string &suffix = "");

    // Maps variables to their offset relative to the base pointer.
    std::map<ast::Base *, int> variable_declarations;

    // Returns the location of a previously defined variable.
    std::string variable(ast::Base *var);

    // Returns the location where the caller places the value for parameter i.
    std::string parameter(std::size_t arg);

    // Handle assignment AST nodes.
    void handleAssignment(ast::BinaryOpExpr &node);

    // ASSIGNMENT: Add any helper functions you use here (if any).
    
    // 8 bytes on top of the stack were added as padding
    bool padded;
};
} // namespace codegen_x64

#endif /* end of include guard: CODEGEN_X64_HPP */
