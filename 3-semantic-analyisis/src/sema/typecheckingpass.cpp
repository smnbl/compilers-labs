#include "sema/collectfuncdeclspass.hpp"
#include "sema/semanticexception.hpp"
#include "sema/typecheckingpass.hpp"
#include "sema/util.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"

#include <algorithm>
#include <fmt/core.h>

#define DEBUG_TYPE "typecheckingpass"

sema::TypeCheckingPass::TypeCheckingPass(llvm::LLVMContext &ctx)
    : ctx(ctx) {
    T_void = sema::Util::parseLLVMType(ctx, "void");
    T_int = sema::Util::parseLLVMType(ctx, "int");
    T_float = sema::Util::parseLLVMType(ctx, "float");
    T_string = sema::Util::parseLLVMType(ctx, "string");
}

llvm::Type *sema::TypeCheckingPass::visitFuncDecl(ast::FuncDecl &node) {

    for (const auto &arg : node.arguments)
        visit(*arg);
    visit(*node.body);


    return nullptr; // No need to return anything here.
}

llvm::Type *sema::TypeCheckingPass::visitIfStmt(ast::IfStmt &node) {
    // ASSIGNMENT: Implement type checking for if statements here.
    visit(*node.condition);
    visit(*node.if_clause);
    if (node.else_clause)
        visit(*node.else_clause);

    return nullptr; // Statements do not have a type.
}

llvm::Type *sema::TypeCheckingPass::visitWhileStmt(ast::WhileStmt &node) {
    // ASSIGNMENT: Implement type checking for while statements here.
    visit(*node.condition);
    visit(*node.body);

    return nullptr; // Statements do not have a type.
}

llvm::Type *sema::TypeCheckingPass::visitReturnStmt(ast::ReturnStmt &node) {
    // ASSIGNMENT: Implement type checking for return statements here.

    if (node.value)
        visit(*node.value);

    return nullptr; // Statements do not have a type.
}

llvm::Type *sema::TypeCheckingPass::visitVarDecl(ast::VarDecl &node) {
    // ASSIGNMENT: Implement type checking for variable declarations here.
    if (node.init)
        visit(*node.init);

    return type_table[&node] = nullptr;
}

llvm::Type *sema::TypeCheckingPass::visitArrayDecl(ast::ArrayDecl &node) {
    visit(*node.size);

    llvm::Type *elem_type = sema::Util::parseLLVMType(ctx, node.type);
    llvm::Type *array_type = llvm::ArrayType::get(elem_type, node.size->value);

    return type_table[&node] = array_type; // Store the type of the array
                                           // declaration in the type table.
}

llvm::Type *sema::TypeCheckingPass::visitBinaryOpExpr(ast::BinaryOpExpr &node) {
    // ASSIGNMENT: Implement type checking for binary operators here.
    visit(*node.lhs);
    visit(*node.rhs);

    return type_table[&node] = nullptr;
}

llvm::Type *sema::TypeCheckingPass::visitUnaryOpExpr(ast::UnaryOpExpr &node) {
    // ASSIGNMENT: Implement type checking for unary operators here.
    visit(*node.operand);

    return type_table[&node] = nullptr;
}

llvm::Type *sema::TypeCheckingPass::visitIntLiteral(ast::IntLiteral &node) {
    return type_table[&node] = T_int;
}

llvm::Type *sema::TypeCheckingPass::visitFloatLiteral(ast::FloatLiteral &node) {
    return type_table[&node] = T_float;
}

llvm::Type *
sema::TypeCheckingPass::visitStringLiteral(ast::StringLiteral &node) {
    return type_table[&node] = T_string;
}

llvm::Type *sema::TypeCheckingPass::visitVarRefExpr(ast::VarRefExpr &node) {
    // ASSIGNMENT: Implement type checking for variable references here.
    return type_table[&node] = nullptr;
}

llvm::Type *sema::TypeCheckingPass::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    // ASSIGNMENT: Implement type checking for array references here.
    visit(*node.index);

    return type_table[&node] = nullptr;
}

llvm::Type *sema::TypeCheckingPass::visitFuncCallExpr(ast::FuncCallExpr &node) {
    // Get the type of the function being called
    auto it = function_table.find(node.name.lexeme);

    if (it == std::end(function_table))
        throw SemanticException(
            fmt::format("Call to unknown function '{}'", node.name.lexeme),
            node.name.begin);

    llvm::FunctionType *funcTy = llvm::cast<llvm::FunctionType>(it->second);

    // Check if the number of arguments matches
    const unsigned int expectedParamSize = funcTy->getNumParams();
    const unsigned int actualParamSize = node.arguments.size();

    if (expectedParamSize != actualParamSize)
        throw SemanticException(
            fmt::format("Invalid number of arguments for call to '{}': {} "
                        "given, but expected {}",
                        node.name.lexeme, actualParamSize, expectedParamSize),
            node.name.begin);

    // Check if argument types match
    for (unsigned int param = 0; param < actualParamSize; ++param) {
        llvm::Type *expected = funcTy->getParamType(param);
        llvm::Type *actual = visit(*node.arguments[param]);

        if (expected != actual)
            throw SemanticException(
                fmt::format("Invalid type for argument {} of call to '{}': {} "
                            "given, but expected {}",
                            param, node.name.lexeme,
                            sema::Util::llvm_type_to_string(actual),
                            sema::Util::llvm_type_to_string(expected)),
                node.name.begin);
    }

    // Result type is the return type of the function
    return type_table[&node] = funcTy->getReturnType();
}
