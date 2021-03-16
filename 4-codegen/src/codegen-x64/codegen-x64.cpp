#include "codegen-x64/codegen-x64.hpp"
#include "codegen-x64/codegenexception.hpp"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <fmt/core.h>

#define DEBUG_TYPE "codegen-x64"

void codegen_x64::CodeGeneratorX64::visitFuncDecl(ast::FuncDecl &node) {
    const std::string name = node.name.lexeme;

    // Clear variable declarations
    variable_declarations.clear();

    // Create a basic block for the function entry
    module << BasicBlock{
        name, fmt::format("Entry point of function '{}'", name), true};

    // Store the name of the exit basic block so we can refer to it in
    // ReturnStmt.
    function_exit = fmt::format(".{}.exit", name);

    // ASSIGNMENT: Implement the function prologue here.
    for (int i=0; i<6; i++){
        module << Instruction{"pushq",{abi_callee_saved_regs[5-i]},"push callee save regs"};
    }

    module << Instruction{"pushq", {"%rbp"}, " weird"};

    // store stack pointer in base pointer
    module << Instruction{"movq",{"%rsp","%rbp"}};

    // align stack pointer  (assume no parameters for now !)
    // TODO add padding based on amount of parameters
    module << Instruction{"subq",{"$8", "%rsp"}};
    padded = true;

    // ASSIGNMENT: Implement function parameters here.
    visit(*node.body);

    // Create a basic block for the function exit.
    module << BasicBlock{function_exit,
                         fmt::format("Exit point of function '{}'", name)};

    // ASSIGNMENT: Implement the function epilogue here. 
    module << Instruction{"movq", {"%rbp","%rsp"}, " weird"};
    module << Instruction{"popq",{"%rbp"}};

    for (int i=0; i<6; i++){
        module << Instruction{"popq",{abi_callee_saved_regs[i]},"push callee save regs"};
    }
    module << Instruction{"retq", {}, "Return to the caller [FuncDecl]"};
}

void codegen_x64::CodeGeneratorX64::visitIfStmt(ast::IfStmt &node) {
    // ASSIGNMENT: Implement if and if-else statements here.
    visit(*node.condition);

    module << Instruction{"popq", {"%r12"}, "pop 1 or 0"};
    std::string val1 = fmt::format("${}", 1);
    module << Instruction{"cmpq", {val1,"%r12"},"check statement"};
    module << Instruction{"jne",{"else"},"jmp to else claus if not equal"};
    visit(*node.if_clause);
    auto unique_label = label(".L0else");
    module << BasicBlock{unique_label,"make new bb"};
    if (node.else_clause)
        visit(*node.else_clause);
}

void codegen_x64::CodeGeneratorX64::visitWhileStmt(ast::WhileStmt &node) {
    // ASSIGNMENT: Implement while statements here.
    auto whilelbl = label("while");
    module << BasicBlock{whilelbl,"new label for while cond"};
    visit(*node.condition);
    module << Instruction{"popq", {"%rax"}, "pop 1 or 0"};
    std::string val1 = fmt::format("${}", 1);
    module << Instruction{"cmpq", {"%rax",val1},"check statement"};
    module << Instruction{"jne",{"endwhile"},"jmp to endwhile clause if not equal"};
    visit(*node.body);
    module << Instruction{"jmp", {"while"}};
    auto endlbl = label("endwhile");
    module << BasicBlock{endlbl,"new label for ending while lp"};
}

void codegen_x64::CodeGeneratorX64::visitReturnStmt(ast::ReturnStmt &node) {
    if (node.value) {
        visit(*node.value);

        module << Instruction{
            "popq",
            {abi_return_reg},
            "Pop return value into return register [ReturnStmt]"};
    }

    module << Instruction{
        "jmp", {function_exit}, "Jump to function exit [ReturnStmt]"};
}

void codegen_x64::CodeGeneratorX64::visitExprStmt(ast::ExprStmt &node) {
    visit(*node.expr);
    module << Instruction{
        "popq", {"%rax"}, "Discard expression statement [ExprStmt]"};
}

void codegen_x64::CodeGeneratorX64::visitVarDecl(ast::VarDecl &node) {
    // if init, initialize with value
    if (node.init) {
        if (padded)
            module << Instruction{"addq", {"$8", "%rsp"}, "fill up padding space"};
        visit(*node.init);
    } else {
        // fill up padding space with variable if padded,
        // otherwise allocate space
        if (!padded)
            module << Instruction{"subq",{"$8","%rsp"}, "make space for variable"};
    }

    // align bytes, add padding if was not padded
    if(!padded) {
        module << Instruction{"subq",{"$8","%rsp"},"align stack again w 16 B"};
        padded = true;
    } else {
        padded = false;
    }

    // put location of the variable relative to the base pointer to the variable declarations member variable
    variable_declarations.insert({&node , -(1+ variable_declarations.size())*8});
}

void codegen_x64::CodeGeneratorX64::visitArrayDecl(ast::ArrayDecl &node) {
    throw CodegenException(
        "Array declarations are not supported by the X64 code generator");

    visit(*node.size);
}

void codegen_x64::CodeGeneratorX64::visitBinaryOpExpr(ast::BinaryOpExpr &node) {
    // We need to handle assignment differently.
    if (node.op.type == TokenType::EQUALS) {
        handleAssignment(node);
        return;
    }

    // ASSIGNMENT: Implement binary operators here.
    visit(*node.lhs);
    visit(*node.rhs);
    std::string val = fmt::format("${}", 0);
    std::string val1 = fmt::format("${}", 1);
    

    module << Instruction{"popq", {"%rax"}, "pop rhs first"};
    module << Instruction{"popq", {"%r15"}, "pop lhs second"};
    switch (node.op.type){
        case (TokenType::PLUS) : //ok
            module << Instruction{"addq", {"%r15","%rax"}, "take sum"};
            module << Instruction{"pushq",{"%rax"}, "push sol on stack of binary op"};
            break;
        case (TokenType::MINUS) : //ok
            module << Instruction{"subq", {"%rax","%r15"}, "take minus, check if this is correct, and not other way round"};
            module << Instruction{"pushq",{"%r15"}, "push sol on stack of binary op"};
            break;
        case (TokenType::STAR) : //ok
            module << Instruction{"imulq", {"%r15"}, "multiply binary"};
            module << Instruction{"pushq",{"%rax"}, "push sol on stack of binary op"};
            break;
        case (TokenType::SLASH) : //not okay
            module << Instruction{"idiv", {"%r15"}, "divide"};
            module << Instruction{"pushq",{"%rax"}, "push sol on stack of binary op"};
            break;
        case (TokenType::PERCENT) : //not okay
            module << Instruction{"idiv", {"%r15"}, "first division"};
            module << Instruction{"pushq",{"%rdx"}, "push sol on stack of binary op"};
            break;
        case (TokenType::BANG_EQUALS) : //ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmove",{"%r12","%r13"}," if r15 and r14 equal, put 0 in r13[bangeq]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
        case (TokenType::EQUALS_EQUALS) : //ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmovne",{"%r12","%r13"}," if r15 and r14 not equal, put 0 in r13[eqeq]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
        case (TokenType::LESS_THAN) : //ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmovae",{"%r12","%r13"},"if higher or equal, move 0 to r13, [less]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
        case (TokenType::LESS_THAN_EQUALS) : // ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmova",{"%r12","%r13"},"if higher, move 0 to r13, [less, or eq]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
        case (TokenType::GREATER_THAN) ://ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmovbe",{"%r12","%r13"},"if lower or equal, move 0 to r13, [greater]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
        case (TokenType::GREATER_THAN_EQUALS) ://ok
            module << Instruction{"movq", {val, "%r12"}, "move 0 to r12 "};
            module << Instruction{"movq", {val1, "%r13"}, "move 1 to r13 "};
            module << Instruction{"cmpq", {"%rax","%r15"}, "check equal, check if this is correct, and not other way round"};
            module << Instruction{"cmovb",{"%r12","%r13"},"if lower, move 0 to r13, [greator eq]"};
            module << Instruction{"pushq",{"%r13"},"push solution"};
            break;
            
    }
    

}

void codegen_x64::CodeGeneratorX64::visitUnaryOpExpr(ast::UnaryOpExpr &node) {
    // ASSIGNMENT: Implement unary operators here. OK!!!!!
    visit(*node.operand);
    std::string val = fmt::format("${}", 1);
    module << Instruction{"movq", {val, "%r12"}, "move 1 to r12 "};
    
    module << Instruction{"popq", {"%r15"}, "pop int for unaryop"};
    if(node.op.type == TokenType::PLUS){
        module << Instruction{"addq", {"%r12","%r15"}, "push int for unary"};
    }
    else{
        module << Instruction{"subq", {"%r12","%r15"}, "mult with -1"};
        
    }
    module << Instruction{"pushq", {"%r15"}, "push int for unary"};
    
}

void codegen_x64::CodeGeneratorX64::visitIntLiteral(ast::IntLiteral &node) {
    std::string val = fmt::format("${}", node.value);
    module << Instruction{"pushq", {val}, "Push integer literal [IntLiteral]"};
}

void codegen_x64::CodeGeneratorX64::visitFloatLiteral(ast::FloatLiteral &node) {
    throw CodegenException(
        "Float literals are not supported by the X64 code generator");
}

void codegen_x64::CodeGeneratorX64::visitStringLiteral(
    ast::StringLiteral &node) {
    throw CodegenException(
        "String literals are not supported by the X64 code generator");
}

void codegen_x64::CodeGeneratorX64::visitVarRefExpr(ast::VarRefExpr &node) {
    // ASSIGNMENT: Implement variable references here.
    std::string address  = variable(symbol_table[&node]);
    module << Instruction{"movq", {address, "%r13"}, "load var in r13"};
    module << Instruction{"pushq",{"%r13"},"push var on stack"};
}

void codegen_x64::CodeGeneratorX64::visitArrayRefExpr(ast::ArrayRefExpr &node) {
    throw CodegenException(
        "Array references are not supported by the X64 code generator");
    visit(*node.index);
}

void codegen_x64::CodeGeneratorX64::visitFuncCallExpr(ast::FuncCallExpr &node) {
    // ASSIGNMENT: Implement function calls here.
    std::string name = fmt::format("{}", node.name.lexeme);
    int i = 0;

    for (const auto &arg : node.arguments){
        //if arg.type
        visit(*arg);
        if(i<6){
            module << Instruction{"popq",{abi_param_regs[i]}};
        }
        i+=1;
    }

    module << Instruction{"call",{name},"call the function after pushing all args"};
    

    while (i>6){
        module << Instruction{"popq",{"%r12"},"call the function after pushing all args"};
        i=i-1;
        
    }
    //if it has return value
    module << Instruction{"pushq",{abi_return_reg}, "put return of function on stack"};
    
}

std::string codegen_x64::CodeGeneratorX64::label(const std::string &suffix) {
    return fmt::format(".L{}{}", label_counter++, suffix);
}

std::string codegen_x64::CodeGeneratorX64::variable(ast::Base *var) {
    auto it = variable_declarations.find(var);

    if (it == std::end(variable_declarations))
        throw CodegenException("Variable is not allocated in the stack frame!");

    return fmt::format("{}(%rbp)", it->second);
}

std::string codegen_x64::CodeGeneratorX64::parameter(std::size_t arg) {
    // ASSIGNMENT: Return the location where the caller places the value for
    // argument 'arg' (0-indexed).
    return "$0";
}

void codegen_x64::CodeGeneratorX64::handleAssignment(ast::BinaryOpExpr &node) {
    // Ensure the LHS is a variable reference
    if (node.lhs->kind != ast::Base::Kind::VarRefExpr) {
        throw CodegenException(
            "Only variable references can be used as lvalues!");
    }

    // Find the declaration of this variable reference
    ast::VarRefExpr &lhs = static_cast<ast::VarRefExpr &>(*node.lhs);
    ast::Base *decl = symbol_table[&lhs];

    // Emit the right hand side
    visit(*node.rhs);

    // Move the right hand side into the variable
    module << Instruction{
        "movq", {"(%rsp)", "%rax"}, "Load RHS of assignment [BinaryOpExpr]"};
    module << Instruction{
        "movq", {"%rax", variable(decl)}, "Assign variable [BinaryOpExpr]"};

    // Note that we do not pop the RHS from the stack, because the value of an
    // assignment is its right-hand side.
}
