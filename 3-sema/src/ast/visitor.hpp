#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "ast/ast.hpp"

#include <cassert>

namespace ast {

template <typename Derived, typename RetTy = void, typename... ArgTys>
struct Visitor {
  private:
    Derived &derived() { return *static_cast<Derived *>(this); }

  public:
    RetTy visitProgram(Program &node, ArgTys... args) {
        for (const auto &decl : node.declarations)
            visit(*decl, args...);

        return RetTy();
    }

    RetTy visitFuncDecl(FuncDecl &node, ArgTys... args) {
        for (const auto &arg : node.arguments)
            visit(*arg, args...);
        visit(*node.body, args...);

        return RetTy();
    }

    RetTy visitEmptyStmt(EmptyStmt &node, ArgTys... args) {
        return RetTy();
    }

    RetTy visitIfStmt(IfStmt &node, ArgTys... args) {
        visit(*node.condition, args...);
        visit(*node.if_clause, args...);
        if (node.else_clause)
            visit(*node.else_clause, args...);

        return RetTy();
    }

    RetTy visitWhileStmt(WhileStmt &node, ArgTys... args) {
        visit(*node.condition, args...);
        visit(*node.body, args...);

        return RetTy();
    }

    RetTy visitReturnStmt(ReturnStmt &node, ArgTys... args) {
        if (node.value)
            visit(*node.value, args...);

        return RetTy();
    }

    RetTy visitExprStmt(ExprStmt &node, ArgTys... args) {
        visit(*node.expr, args...);

        return RetTy();
    }

    RetTy visitVarDecl(VarDecl &node, ArgTys... args) {
        if(node.init)
            visit(*node.init, args...);

        return RetTy();
    }

    RetTy visitArrayDecl(ArrayDecl &node, ArgTys... args) {
        visit(*node.size, args...);

        return RetTy();
    }

    RetTy visitCompoundStmt(CompoundStmt &node, ArgTys... args) {
        for (const auto &stmt : node.body)
            visit(*stmt, args...);

        return RetTy();
    }

    RetTy visitBinaryOpExpr(BinaryOpExpr &node, ArgTys... args) {
        visit(*node.lhs, args...);
        visit(*node.rhs, args...);

        return RetTy();
    }

    RetTy visitUnaryOpExpr(UnaryOpExpr &node, ArgTys... args) {
        visit(*node.operand, args...);

        return RetTy();
    }

    RetTy visitIntLiteral(IntLiteral &node, ArgTys... args) {
        return RetTy();
    }

    RetTy visitFloatLiteral(FloatLiteral &node, ArgTys... args) {
        return RetTy();
    }

    RetTy visitStringLiteral(StringLiteral &node, ArgTys... args) {
        return RetTy();
    }

    RetTy visitVarRefExpr(VarRefExpr &node, ArgTys... args) {
        return RetTy();
    }

    RetTy visitArrayRefExpr(ArrayRefExpr &node, ArgTys... args) {
        visit(*node.index, args...);

        return RetTy();
    }

    RetTy visitFuncCallExpr(FuncCallExpr &node, ArgTys... args) {
        for (const auto& arg : node.arguments)
            visit(*arg, args...);

        return RetTy();
    }

    RetTy visit(Base &node, ArgTys... args) {
        switch (node.kind) {
        case Base::Kind::Program:
            return derived().visitProgram(static_cast<Program &>(node),
                                          args...);
        case Base::Kind::FuncDecl:
            return derived().visitFuncDecl(static_cast<FuncDecl &>(node),
                                           args...);
        case Base::Kind::EmptyStmt:
            return derived().visitEmptyStmt(static_cast<EmptyStmt &>(node),
                                            args...);
        case Base::Kind::IfStmt:
            return derived().visitIfStmt(static_cast<IfStmt &>(node), args...);
        case Base::Kind::WhileStmt:
            return derived().visitWhileStmt(static_cast<WhileStmt &>(node),
                                            args...);
        case Base::Kind::ReturnStmt:
            return derived().visitReturnStmt(static_cast<ReturnStmt &>(node),
                                             args...);
        case Base::Kind::ExprStmt:
            return derived().visitExprStmt(static_cast<ExprStmt &>(node),
                                           args...);
        case Base::Kind::VarDecl:
            return derived().visitVarDecl(static_cast<VarDecl &>(node),
                                          args...);
        case Base::Kind::ArrayDecl:
            return derived().visitArrayDecl(static_cast<ArrayDecl &>(node),
                                            args...);
        case Base::Kind::CompoundStmt:
            return derived().visitCompoundStmt(
                static_cast<CompoundStmt &>(node), args...);
        case Base::Kind::BinaryOpExpr:
            return derived().visitBinaryOpExpr(
                static_cast<BinaryOpExpr &>(node), args...);
        case Base::Kind::UnaryOpExpr:
            return derived().visitUnaryOpExpr(static_cast<UnaryOpExpr &>(node),
                                              args...);
        case Base::Kind::IntLiteral:
            return derived().visitIntLiteral(static_cast<IntLiteral &>(node),
                                             args...);
        case Base::Kind::FloatLiteral:
            return derived().visitFloatLiteral(
                static_cast<FloatLiteral &>(node), args...);
        case Base::Kind::StringLiteral:
            return derived().visitStringLiteral(
                static_cast<StringLiteral &>(node), args...);
        case Base::Kind::VarRefExpr:
            return derived().visitVarRefExpr(static_cast<VarRefExpr &>(node),
                                             args...);
        case Base::Kind::ArrayRefExpr:
            return derived().visitArrayRefExpr(
                static_cast<ArrayRefExpr &>(node), args...);
        case Base::Kind::FuncCallExpr:
            return derived().visitFuncCallExpr(
                static_cast<FuncCallExpr &>(node), args...);
        default:
            assert(false && "Unhandled AST type in visitor!");
        }
    }
};

} // namespace ast

#endif /* end of include guard: AST_VISITOR_HPP */
