#include "ast/prettyprinter.hpp"

#include <sstream>

ast::PrettyPrinter::FieldsStream
ast::PrettyPrinter::printHelper(std::string &indent, bool last,
                                const std::string &name) {
    os << indent;

    if (last) {
        os << (ascii ? "`-- " : "└── ");
        indent += "    ";
    } else {
        os << (ascii ? "|-- " : "├── ");
        indent += (ascii ? "|   " : "│   ");
    }

    os << name;

    return FieldsStream(os);
}

std::string ast::PrettyPrinter::endNode(const ast::Base &node) {
    std::ostringstream oss;

    if (dump_ids)
        oss << " <" << node.id << ">";

    oss << "\n";

    return oss.str();
}

void ast::PrettyPrinter::visitProgram(Program &node, std::string indent,
                                      bool last) {
    printHelper(indent, last, "Program") << endNode(node);

    for (std::size_t i = 0; i < node.declarations.size(); ++i)
        visit(*node.declarations[i], indent, i == node.declarations.size() - 1);
}

void ast::PrettyPrinter::visitFuncDecl(FuncDecl &node, std::string indent,
                                       bool last) {
    printHelper(indent, last, "FuncDecl")
        << std::make_pair("returnType", node.returnType.lexeme)
        << std::make_pair("name", node.name.lexeme) << endNode(node);

    for (std::size_t i = 0; i < node.arguments.size(); ++i)
        visit(*node.arguments[i], indent, false);

    visit(*node.body, indent, true);
}

void ast::PrettyPrinter::visitEmptyStmt(EmptyStmt &node, std::string indent,
                                        bool last) {
    printHelper(indent, last, "EmptyStmt") << endNode(node);
}

void ast::PrettyPrinter::visitIfStmt(IfStmt &node, std::string indent,
                                     bool last) {
    printHelper(indent, last, "IfStmt") << endNode(node);

    visit(*node.condition, indent, false);
    visit(*node.if_clause, indent, node.else_clause == nullptr);

    if (node.else_clause)
        visit(*node.else_clause, indent, true);
}

void ast::PrettyPrinter::visitWhileStmt(WhileStmt &node, std::string indent,
                                        bool last) {
    printHelper(indent, last, "WhileStmt") << endNode(node);

    visit(*node.condition, indent, false);
    visit(*node.body, indent, true);
}

void ast::PrettyPrinter::visitReturnStmt(ReturnStmt &node, std::string indent,
                                         bool last) {
    printHelper(indent, last, "ReturnStmt") << endNode(node);

    if (node.value)
        visit(*node.value, indent, true);
}

void ast::PrettyPrinter::visitExprStmt(ExprStmt &node, std::string indent,
                                       bool last) {
    printHelper(indent, last, "ExprStmt") << endNode(node);

    visit(*node.expr, indent, true);
}

void ast::PrettyPrinter::visitVarDecl(VarDecl &node, std::string indent,
                                      bool last) {
    printHelper(indent, last, "VarDecl")
        << std::make_pair("type", node.type.lexeme)
        << std::make_pair("name", node.name.lexeme) << endNode(node);

    if (node.init)
        visit(*node.init, indent, true);
}

void ast::PrettyPrinter::visitArrayDecl(ArrayDecl &node, std::string indent,
                                        bool last) {
    printHelper(indent, last, "ArrayDecl")
        << std::make_pair("type", node.type.lexeme)
        << std::make_pair("name", node.name.lexeme) << endNode(node);

    visit(*node.size, indent, true);
}

void ast::PrettyPrinter::visitCompoundStmt(CompoundStmt &node,
                                           std::string indent, bool last) {
    printHelper(indent, last, "CompoundStmt") << endNode(node);

    for (std::size_t i = 0; i < node.body.size(); ++i)
        visit(*node.body[i], indent, i == node.body.size() - 1);
}

void ast::PrettyPrinter::visitBinaryOpExpr(BinaryOpExpr &node,
                                           std::string indent, bool last) {
    printHelper(indent, last, "BinaryOpExpr")
        << std::make_pair("op", node.op.lexeme) << endNode(node);

    visit(*node.lhs, indent, false);
    visit(*node.rhs, indent, true);
}

void ast::PrettyPrinter::visitUnaryOpExpr(UnaryOpExpr &node, std::string indent,
                                          bool last) {
    printHelper(indent, last, "UnaryOpExpr")
        << std::make_pair("op", node.op.lexeme) << endNode(node);

    visit(*node.operand, indent, true);
}

void ast::PrettyPrinter::visitIntLiteral(IntLiteral &node, std::string indent,
                                         bool last) {
    printHelper(indent, last, "IntLiteral")
        << std::make_pair("value", node.value) << endNode(node);
}

void ast::PrettyPrinter::visitFloatLiteral(FloatLiteral &node,
                                           std::string indent, bool last) {
    printHelper(indent, last, "FloatLiteral")
        << std::make_pair("value", node.value) << endNode(node);
}

void ast::PrettyPrinter::visitStringLiteral(StringLiteral &node,
                                            std::string indent, bool last) {
    printHelper(indent, last, "StringLiteral")
        << std::make_pair("value", node.value) << endNode(node);
}

void ast::PrettyPrinter::visitVarRefExpr(VarRefExpr &node, std::string indent,
                                         bool last) {
    printHelper(indent, last, "VarRefExpr")
        << std::make_pair("name", node.name.lexeme) << endNode(node);
}

void ast::PrettyPrinter::visitArrayRefExpr(ArrayRefExpr &node,
                                           std::string indent, bool last) {
    printHelper(indent, last, "ArrayRefExpr")
        << std::make_pair("name", node.name.lexeme) << endNode(node);

    visit(*node.index, indent, true);
}

void ast::PrettyPrinter::visitFuncCallExpr(FuncCallExpr &node,
                                           std::string indent, bool last) {
    printHelper(indent, last, "FuncCallExpr")
        << std::make_pair("name", node.name.lexeme) << endNode(node);

    for (std::size_t i = 0; i < node.arguments.size(); ++i)
        visit(*node.arguments[i], indent, i == node.arguments.size() - 1);
}
