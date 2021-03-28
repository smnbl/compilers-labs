#include "codegen-llvm/codegen-llvm.hpp"
#include "codegen-llvm/codegenexception.hpp"
#include "sema/util.hpp"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <fmt/core.h>

#define DEBUG_TYPE "codegen-llvm"

codegen_llvm::CodeGeneratorLLVM::CodeGeneratorLLVM(
    llvm::LLVMContext &ctx,
    const sema::CollectFuncDeclsPass::FunctionTable &function_table,
    const sema::ScopeResolutionPass::SymbolTable &symbol_table,
    const sema::TypeCheckingPass::TypeTable &type_table)
    : context(ctx), builder(ctx),
      module(std::make_unique<llvm::Module>("microcc-module", ctx)),
      function_table(function_table), symbol_table(symbol_table),
      type_table(type_table) {
    // Initialise LLVM types.
    T_void = sema::Util::parseLLVMType(ctx, "void");
    T_int = sema::Util::parseLLVMType(ctx, "int");
    T_float = sema::Util::parseLLVMType(ctx, "float");
    T_string = sema::Util::parseLLVMType(ctx, "string");

    // Add declarations for all functions that are used (including those in the
    // micro-C runtime).
    for (const auto &entry : function_table) {
        const auto &name = entry.first;
        const auto &type = llvm::cast<llvm::FunctionType>(entry.second);

        llvm::Function *func = llvm::Function::Create(
            type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name,
            *module);
    }
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitFuncDecl(ast::FuncDecl &node) {
    // Get the function declaration from the module.
    llvm::Function *func = module->getFunction(node.name.lexeme);
    if (!func)
        throw CodegenException(fmt::format(
            "Did not find function '{}' in the LLVM module symbol table!",
            node.name.lexeme));

    // Create an entry basic block in the function.
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);

    // Set the insertion point of the builder to the end of this basic block.
    builder.SetInsertPoint(entry);


    // Generate code for the body of the function.
    visit(*node.body);

    // Check if we need to add an implicit return, in case the user didn't add
    // any.
    if (!isCurrentBasicBlockTerminated()) {
        llvm::Type *retTy = func->getReturnType();
        if (retTy == T_void)
            builder.CreateRetVoid();
        else
            builder.CreateRet(llvm::Constant::getNullValue(retTy));
    }

    // Validate the generated code, checking for consistency.
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        throw CodegenException(
            fmt::format("Function '{}' failed validation", node.name.lexeme));
    }

    return func;
}

llvm::Value *codegen_llvm::CodeGeneratorLLVM::visitIfStmt(ast::IfStmt &node) {
    // ASSIGNMENT: Implement if statements here.
    throw CodegenException("ASSIGNMENT: if statements are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitWhileStmt(ast::WhileStmt &node) {
    // ASSIGNMENT: Implement while statements here.
    throw CodegenException("ASSIGNMENT: while statements are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitReturnStmt(ast::ReturnStmt &node) {
    if (node.value) {
        // return with value
        llvm::Value *retVal = visit(*node.value);
        return builder.CreateRet(retVal);
    } else {
        // void return
        return builder.CreateRetVoid();
    }
}

llvm::Value *codegen_llvm::CodeGeneratorLLVM::visitVarDecl(ast::VarDecl &node) {
    // ASSIGNMENT: Implement variable declarations here.
    throw CodegenException(
        "ASSIGNMENT: variable declarations are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitArrayDecl(ast::ArrayDecl &node) {
    // ASSIGNMENT: Implement array declarations here.
    throw CodegenException(
        "ASSIGNMENT: array declarations are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitBinaryOpExpr(ast::BinaryOpExpr &node) {
    // ASSIGNMENT: Implement binary operators here.
    throw CodegenException("ASSIGNMENT: binary operators are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitUnaryOpExpr(ast::UnaryOpExpr &node) {
    // ASSIGNMENT: Implement unary operators here.
    throw CodegenException("ASSIGNMENT: unary operators are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitIntLiteral(ast::IntLiteral &node) {
    return llvm::ConstantInt::get(T_int, node.value);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitFloatLiteral(ast::FloatLiteral &node) {
    return llvm::ConstantFP::get(T_float, node.value);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitStringLiteral(ast::StringLiteral &node) {
    return builder.CreateGlobalStringPtr(node.value);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitVarRefExpr(ast::VarRefExpr &node) {
    // ASSIGNMENT: Implement variable references here.
    throw CodegenException(
        "ASSIGNMENT: variable references are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    // ASSIGNMENT: Implement array references here.
    throw CodegenException("ASSIGNMENT: array references are not implemented!");
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitFuncCallExpr(ast::FuncCallExpr &node) {
    // ASSIGNMENT: Implement function calls here.
    throw CodegenException("ASSIGNMENT: function calls are not implemented!");
}

llvm::AllocaInst *codegen_llvm::CodeGeneratorLLVM::createAllocaInEntryBlock(
    llvm::Type *type, llvm::Value *array_size) {
    // Get a reference to the current function.
    llvm::Function *current_function = builder.GetInsertBlock()->getParent();

    // Get a reference to the current function's entry block.
    llvm::BasicBlock &entry_block = current_function->getEntryBlock();

    // Create the alloca
    llvm::IRBuilder<> alloca_builder{&entry_block, entry_block.begin()};
    return alloca_builder.CreateAlloca(type, array_size);
}

bool codegen_llvm::CodeGeneratorLLVM::isCurrentBasicBlockTerminated() {
    return builder.GetInsertBlock()->getTerminator() != nullptr;
}

