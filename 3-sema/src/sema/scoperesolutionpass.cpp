#include "sema/scoperesolutionpass.hpp"
#include "sema/semanticexception.hpp"

#include "llvm/Support/Debug.h"

#include <fmt/core.h>
#include <stdexcept>

#define DEBUG_TYPE "scoperesolutionpass"

void sema::ScopeResolutionPass::pushScope() { scopes.emplace_back(); }

void sema::ScopeResolutionPass::popScope() { scopes.pop_back(); }

bool sema::ScopeResolutionPass::isDefined(const std::string &name) const {
    if (scopes.empty())
        throw SemanticException("Scopes stack is empty!");
    return scopes.back().find(name) != std::end(scopes.back());
}

void sema::ScopeResolutionPass::define(const std::string &name,
                                       ast::Base *node) {
    // Check if variable is already defined
    if (isDefined(name))
        throw SemanticException(
            fmt::format("Cannot redefine variable '{}'", name));

    scopes.back().emplace(std::make_pair(name, node));
}

ast::Base *sema::ScopeResolutionPass::resolve(const std::string &name) const {
    for (auto it = std::rbegin(scopes); it != std::rend(scopes); ++it) {
        auto var_it = it->find(name);

        if (var_it != std::end(*it))
            return var_it->second;
    }

    throw SemanticException(fmt::format("Undefined variable '{}'", name));
}

void sema::ScopeResolutionPass::visitProgram(ast::Program &node) {
    // Open global scope
    pushScope();

    for (const auto &decl : node.declarations)
        visit(*decl);

    // Close global scope
    popScope();
}

void sema::ScopeResolutionPass::visitFuncDecl(ast::FuncDecl &node) {
    // ASSIGNMENT: Implement scope resolution.

    // Open function scope (arguments)
    pushScope();

    for (const auto &arg : node.arguments)
        visit(*arg);

    visit(*node.body);

    // Pop function scope (arguments)
    // this will open a new scope for the body (can shadow function arguments)
    popScope();
}

void sema::ScopeResolutionPass::visitIfStmt(ast::IfStmt &node) {
    visit(*node.condition);

    pushScope();
    visit(*node.if_clause);
    popScope();

    if (node.else_clause) {
        pushScope();
        visit(*node.else_clause);
        popScope();
    }
}

void sema::ScopeResolutionPass::visitWhileStmt(ast::WhileStmt &node) {
    visit(*node.condition);

    pushScope();
    visit(*node.body);
    popScope();
}

void sema::ScopeResolutionPass::visitVarDecl(ast::VarDecl &node) {
    // visit right hand side first!
    if (node.init)
        visit(*node.init);

    define(node.name.lexeme, &node);
}

void sema::ScopeResolutionPass::visitArrayDecl(ast::ArrayDecl &node) {
    // visit right hand side first!
    visit(*node.size);

    define(node.name.lexeme, &node);
}

void sema::ScopeResolutionPass::visitCompoundStmt(ast::CompoundStmt &node) {
    pushScope();

    for (const auto &stmt : node.body)
        visit(*stmt);

    popScope();
}

void sema::ScopeResolutionPass::visitVarRefExpr(ast::VarRefExpr &node) {
    auto definition = resolve(node.name.lexeme);
    symbol_table[&node] = definition;
}

void sema::ScopeResolutionPass::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    auto definition = resolve(node.name.lexeme);
    symbol_table[&node] = definition;

    visit(*node.index);
}
