#ifndef SCOPERESOLUTIONPASS_HPP
#define SCOPERESOLUTIONPASS_HPP

#include "ast/ast.hpp"
#include "ast/visitor.hpp"

#include <deque>
#include <map>
#include <string>

namespace sema {
// AST pass that checks if variables are defined before they are used, and that
// performs scope resolution (i.e. binds uses of variables to their definitions
// according to the scoping rules of micro-C).
class ScopeResolutionPass : public ast::Visitor<ScopeResolutionPass> {
  public:
    using SymbolTable = std::map<ast::Base *, ast::Base *>;

    SymbolTable getSymbolTable() const { return symbol_table; }

    void visitProgram(ast::Program &node);
    void visitFuncDecl(ast::FuncDecl &node);
    void visitIfStmt(ast::IfStmt &node);
    void visitWhileStmt(ast::WhileStmt &node);
    void visitVarDecl(ast::VarDecl &node);
    void visitArrayDecl(ast::ArrayDecl &node);
    void visitCompoundStmt(ast::CompoundStmt &node);
    void visitVarRefExpr(ast::VarRefExpr &node);
    void visitArrayRefExpr(ast::ArrayRefExpr &node);

  private:
    // Maps each use of a variable (or array) to its definition.
    SymbolTable symbol_table;

    // Environment, binding the name of a variable to its AST node.
    using Environment = std::map<std::string, ast::Base *>;

    // Stack of scopes
    std::deque<Environment> scopes;

    // Push a scope at the top of the scope stack.
    void pushScope();

    // Pop the scope at the top of the scope stack.
    void popScope();

    // Returns true if the variable 'name' is defined in the scope at the top of
    // the scope stack.
    bool isDefined(const std::string &name) const;

    // Adds a new entry to the top scope, binding the variable 'name' to the AST
    // node 'node'. Throws an exception if 'name' is already defined in the
    // current scope.
    void define(const std::string &name, ast::Base *node);

    // Gets the AST node corresponding to the definition of the variable 'name'
    // in the innermost scope. Throws an exception if 'name' is not defined in
    // any scope.
    ast::Base *resolve(const std::string &name) const;
};
} // namespace sema

#endif /* end of include guard: SCOPERESOLUTIONPASS_HPP */
