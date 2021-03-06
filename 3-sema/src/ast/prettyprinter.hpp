#ifndef AST_PRETTYPRINTER_H
#define AST_PRETTYPRINTER_H

#include "ast/ast.hpp"
#include "ast/visitor.hpp"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace ast {
class PrettyPrinter
    : public Visitor<PrettyPrinter, void, const std::string &, bool> {
  public:
    PrettyPrinter(std::ostream &os, bool ascii = false, bool dump_ids = false) : os(os), ascii(ascii), dump_ids(dump_ids) {}

    void visitProgram(Program &node, std::string indent, bool last);
    void visitFuncDecl(FuncDecl &node, std::string indent, bool last);
    void visitEmptyStmt(EmptyStmt &node, std::string indent, bool last);
    void visitIfStmt(IfStmt &node, std::string indent, bool last);
    void visitWhileStmt(WhileStmt &node, std::string indent, bool last);
    void visitReturnStmt(ReturnStmt &node, std::string indent, bool last);
    void visitExprStmt(ExprStmt &node, std::string indent, bool last);
    void visitVarDecl(VarDecl &node, std::string indent, bool last);
    void visitArrayDecl(ArrayDecl &node, std::string indent, bool last);
    void visitCompoundStmt(CompoundStmt &node, std::string indent, bool last);
    void visitBinaryOpExpr(BinaryOpExpr &node, std::string indent, bool last);
    void visitUnaryOpExpr(UnaryOpExpr &node, std::string indent, bool last);
    void visitIntLiteral(IntLiteral &node, std::string indent, bool last);
    void visitFloatLiteral(FloatLiteral &node, std::string indent, bool last);
    void visitStringLiteral(StringLiteral &node, std::string indent, bool last);
    void visitVarRefExpr(VarRefExpr &node, std::string indent, bool last);
    void visitArrayRefExpr(ArrayRefExpr &node, std::string indent, bool last);
    void visitFuncCallExpr(FuncCallExpr &node, std::string indent, bool last);

  private:
    class FieldsStream {
      private:
        std::ostream &os;
        bool first = true;

      public:
        FieldsStream(std::ostream &os) : os(os) {}

        template <typename T> FieldsStream &operator<<(const T &t) { os << t; return *this; }

        template <typename U, typename T>
        FieldsStream &operator<<(const std::pair<U, T> &pair) {
            os << (first ? ": " : ", ") << pair.first << " = '" << pair.second
               << "'";
            first = false;

            return *this;
        }
    };

    std::ostream &os;
    bool ascii;
    bool dump_ids;

    FieldsStream printHelper(std::string &indent, bool last, const std::string &name);
    std::string endNode(const ast::Base &node);
};
} // namespace ast

#endif /* end of include guard: AST_PRETTYPRINTER_H */
