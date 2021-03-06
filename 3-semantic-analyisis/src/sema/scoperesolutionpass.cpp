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

    for (const auto &arg : node.arguments)
        visit(*arg);
    visit(*node.body);
}

void sema::ScopeResolutionPass::visitIfStmt(ast::IfStmt &node) {
    // ASSIGNMENT: Implement scope resolution.

    visit(*node.condition);
    visit(*node.if_clause);
    if (node.else_clause)
        visit(*node.else_clause);
}

void sema::ScopeResolutionPass::visitWhileStmt(ast::WhileStmt &node) {
    // ASSIGNMENT: Implement scope resolution.

    visit(*node.condition);
    visit(*node.body);
}

void sema::ScopeResolutionPass::visitVarDecl(ast::VarDecl &node) {
    // ASSIGNMENT: Implement scope resolution.

    if (node.init)
        visit(*node.init);
}

void sema::ScopeResolutionPass::visitArrayDecl(ast::ArrayDecl &node) {
    // ASSIGNMENT: Implement scope resolution.

    visit(*node.size);
}

void sema::ScopeResolutionPass::visitCompoundStmt(ast::CompoundStmt &node) {
    pushScope();

    for (const auto &stmt : node.body)
        visit(*stmt);

    popScope();
}

void sema::ScopeResolutionPass::visitVarRefExpr(ast::VarRefExpr &node) {
    // ASSIGNMENT: Implement scope resolution.
}

void sema::ScopeResolutionPass::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    // ASSIGNMENT: Implement scope resolution.

    visit(*node.index);
}
