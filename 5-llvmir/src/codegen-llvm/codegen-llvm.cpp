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
#include <iostream>

#include <fmt/core.h>
#include <iostream>
using namespace std;

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

    // put function arguments into local variables
    int i = 0;
    for (auto const &parameter : node.arguments) {
        llvm::Value *var = visit(*parameter);
        builder.CreateStore(func->getArg(i++), var);
    }

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

    //Validate the generated code, checking for consistency.
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        throw CodegenException(
            fmt::format("Function '{}' failed validation", node.name.lexeme));
    }

    return func;
}

llvm::Value *codegen_llvm::CodeGeneratorLLVM::visitIfStmt(ast::IfStmt &node) {
    // ASSIGNMENT: Implement if statements here.
    llvm::Function *current_function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *entryif = llvm::BasicBlock::Create(context, "entryif", current_function);
    llvm::BasicBlock *exit = llvm::BasicBlock::Create(context, "exit", current_function);
    llvm::BasicBlock *entryelse;

    if (node.else_clause != nullptr)
        entryelse = llvm::BasicBlock::Create(context, "entryelse", current_function);

    auto cmp = visit(*node.condition);
    auto zero = llvm::ConstantInt::get(T_int, 0);

    llvm::Value* cond = builder.CreateICmpNE(cmp,zero, "cond");

    if (node.else_clause != nullptr)
        builder.CreateCondBr(cond, entryif, entryelse);
    else
        builder.CreateCondBr(cond, entryif, exit);

    builder.SetInsertPoint(entryif);
    visit(*node.if_clause);
    if (!isCurrentBasicBlockTerminated()) {    
        builder.CreateBr(exit);   
    }

    if (node.else_clause != nullptr) {
        builder.SetInsertPoint(entryelse);

        visit(*node.else_clause);
        if (!isCurrentBasicBlockTerminated()) {    
            builder.CreateBr(exit);   
        }
    }


    builder.SetInsertPoint(exit);
    return nullptr;
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitWhileStmt(ast::WhileStmt &node) {
    // ASSIGNMENT: Implement while statements here.
    llvm::Function *current_function = builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *whilebody = llvm::BasicBlock::Create(context, "entrywhile",current_function);
    llvm::BasicBlock *whilecond = llvm::BasicBlock::Create(context, "whilecond", current_function);
    llvm::BasicBlock *whileend = llvm::BasicBlock::Create(context, "whileend", current_function);

    builder.CreateBr(whilecond);
    builder.SetInsertPoint(whilecond);
    auto cmp = visit(*node.condition);
    auto zero = llvm::ConstantInt::get(T_int, 0);

    llvm::Value* cond = builder.CreateICmpNE(cmp, zero, "cond");
    builder.CreateCondBr(cond, whilebody,whileend);
    
    builder.SetInsertPoint(whilebody);
    visit(*node.body);
    builder.CreateBr(whilecond);

    builder.SetInsertPoint(whileend);
    return(nullptr);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitReturnStmt(ast::ReturnStmt &node) {
    if (node.value) {
        // return with value
        llvm::Value* retVal = visit(*node.value);
        return builder.CreateRet(retVal);
    } else {
        // void return
        return builder.CreateRetVoid();
    }
}

// helper function to get the base type, of an array / var decl
llvm::Type *
codegen_llvm::CodeGeneratorLLVM::getBaseType(std::string lexeme) {
    if (lexeme == "int")
            return T_int;
    else 
    if (lexeme == "float")
            return T_float;
    else
    if (lexeme == "string")
            return llvm::Type::getInt8PtrTy(context);
    return nullptr;
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitVarDecl(ast::VarDecl &node) {
    // ASSIGNMENT: Implement variable declarations here.
    llvm::Type* var_type = getBaseType(node.type.lexeme);
    if (var_type == nullptr)
            throw CodegenException("VarDecl not implemented yet for this type!");

    llvm::AllocaInst *ptr = createAllocaInEntryBlock(var_type);
    var_allocas[&node] = ptr;

    // if init, initialize the variable
    if (node.init != nullptr) {
        llvm::Value *value = visit(*node.init);
        builder.CreateStore(value, ptr);
    }

    return ptr;
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitArrayDecl(ast::ArrayDecl &node) {
    // ASSIGNMENT: Implement array declarations here.
    llvm::Type* var_type = getBaseType(node.type.lexeme);
    if (var_type == nullptr)
            throw CodegenException("ArrayDecl not implemented yet for this type!");

    llvm::AllocaInst *ptr = createAllocaInEntryBlock(var_type, visit(*node.size));
    var_allocas[&node] = ptr;

    return nullptr; // TODO, right thing to return?
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitBinaryOpExpr(ast::BinaryOpExpr &node) {
    // ASSIGNMENT: Implement binary operators here.
    llvm::Value *lhs;
    llvm::Value *rhs;

    rhs = visit(*node.rhs);

    // assignments
    if (node.op.type == TokenType::EQUALS) {
            llvm::Value *ptr = var_allocas[symbol_table[(ast::Base*) &(*node.lhs)]];
            switch (node.lhs->kind) {
                case ast::Base::Kind::VarRefExpr:
                    builder.CreateStore(rhs, ptr);
                break;
                case ast::Base::Kind::ArrayRefExpr:
                {
                    auto index = visit(*(((ast::ArrayRefExpr*) &(*node.lhs))->index));
                    builder.CreateStore(rhs, builder.CreateGEP(ptr, index));
                }
                break;
                default:
                    throw CodegenException("unsupported lhs reference");
            }
            return rhs;
    }

    lhs = visit(*node.lhs);

    switch (node.op.type) {
        case (TokenType::CARET):
            if(lhs->getType() == T_int) {
                // cast ints to floats
                lhs = builder.CreateCast(llvm::Instruction::SIToFP, lhs, T_float);
                rhs = builder.CreateCast(llvm::Instruction::SIToFP, rhs, T_float);

                auto res_f = builder.CreateBinaryIntrinsic(llvm::Intrinsic::IndependentIntrinsics::pow, lhs, rhs);

                return builder.CreateCast(llvm::Instruction::FPToSI, res_f, T_int);
            } else {
                return builder.CreateBinaryIntrinsic(llvm::Intrinsic::IndependentIntrinsics::pow, lhs, rhs);
            }
            break;
        case (TokenType::STAR): // ok
            if (lhs->getType() == T_int)
                return builder.CreateMul(lhs, rhs);
            else
                return builder.CreateFMul(lhs, rhs);
            break;
        case (TokenType::SLASH): // ok
            if (lhs->getType() == T_int)
                return builder.CreateSDiv(lhs, rhs);
            else
                return builder.CreateFDiv(lhs, rhs);
            break;
        case (TokenType::PERCENT):
            if (lhs->getType() == T_int)
                return builder.CreateSRem(lhs, rhs); // TODO juiste versie?
            else
                return builder.CreateFRem(lhs, rhs);
            break;
        case (TokenType::PLUS):
            if (lhs->getType() == T_int)
                return builder.CreateAdd(lhs, rhs);
            else
                return builder.CreateFAdd(lhs, rhs);
            break;
        case (TokenType::MINUS):
            if (lhs->getType() == T_int)
                return builder.CreateSub(lhs, rhs);
            else
                return builder.CreateFSub(lhs, rhs);
            break;
        case (TokenType::LESS_THAN):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_SLT, lhs, rhs); 
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_OLT, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        case (TokenType::LESS_THAN_EQUALS):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_SLE, lhs, rhs);
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_OLE, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        case (TokenType::GREATER_THAN):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_SGT, lhs, rhs);
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_OGT, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        case (TokenType::GREATER_THAN_EQUALS):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_SGE, lhs, rhs);
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_OGE, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        case (TokenType::EQUALS_EQUALS):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_EQ, lhs, rhs);
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_OEQ, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        case (TokenType::BANG_EQUALS):
            if (lhs->getType() == T_int)
                return generateCompareInt(llvm::CmpInst::Predicate::ICMP_NE, lhs, rhs);
            else
                return generateCompareFloat(llvm::CmpInst::Predicate::FCMP_ONE, lhs, rhs); // clang seems to use unordered comparisons by default?
            break;
        default:
            throw CodegenException("unsupported binary operator!");
    }
    return nullptr;
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::generateCompareInt(llvm::CmpInst::Predicate predicate, llvm::Value* lhs, llvm::Value* rhs) {
    auto cmp = builder.CreateICmp(predicate, lhs, rhs, "cmp");
    return builder.CreateZExt(cmp, T_int);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::generateCompareFloat(llvm::CmpInst::Predicate predicate, llvm::Value* lhs, llvm::Value* rhs) {
    auto cmp = builder.CreateFCmp(predicate, lhs, rhs, "cmp");
    return builder.CreateZExt(cmp, T_int);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitUnaryOpExpr(ast::UnaryOpExpr &node) {
    // ASSIGNMENT: Implement unary operators here.
    auto operand = visit(*node.operand);

    switch (node.op.type) {
        case(TokenType::PLUS):
            return operand;
            break;
        case(TokenType::MINUS):
            if (operand->getType() == T_int)
                return builder.CreateSub(llvm::ConstantInt::get(T_int, 0), operand);
            else
                return builder.CreateFSub(llvm::ConstantFP::get(T_float, 0), operand);
            break;
        default:
            throw CodegenException("unsupported unary operator!");
    }
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
    llvm::AllocaInst *ptr = var_allocas[symbol_table[(ast::Base*) &(node)]];
    return builder.CreateLoad(ptr->getAllocatedType(), ptr, node.name.lexeme);
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    // ASSIGNMENT: Implement array references here.
    llvm::AllocaInst *ptr = var_allocas[symbol_table[(ast::Base*) &(node)]];
    return builder.CreateLoad(ptr->getAllocatedType(), builder.CreateGEP(ptr, visit(*node.index), node.name.lexeme));
}

llvm::Value *
codegen_llvm::CodeGeneratorLLVM::visitFuncCallExpr(ast::FuncCallExpr &node) {
    // ASSIGNMENT: Implement function calls here.
    std::vector<llvm::Value *> args;

    for (auto const &arg: node.arguments) {
        args.push_back(visit(*arg));
    }

    return builder.CreateCall(module->getFunction(node.name.lexeme), llvm::ArrayRef<llvm::Value*>(args));
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

