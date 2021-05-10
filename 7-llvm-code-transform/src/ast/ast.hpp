#ifndef AST_HPP
#define AST_HPP

#include "lexer/token.hpp"

#include <memory>
#include <vector>

namespace ast {

template <typename T> using List = std::vector<T>;

// NOTE: Using std::unique_ptr<T> instead of std::shared_ptr<T> is more correct,
// semantically speaking, but it requires std::move's everywhere, so let's not
// do it for now.
template <typename T> using Ptr = std::shared_ptr<T>;

// Base class
struct Base {
    const enum class Kind {
        Program,
        FuncDecl,
        EmptyStmt,
        IfStmt,
        WhileStmt,
        ReturnStmt,
        ExprStmt,
        VarDecl,
        ArrayDecl,
        CompoundStmt,
        BinaryOpExpr,
        UnaryOpExpr,
        IntLiteral,
        FloatLiteral,
        StringLiteral,
        VarRefExpr,
        ArrayRefExpr,
        FuncCallExpr
    } kind;

    unsigned int id;

    Base(Kind kind) : kind(kind) {
        static int next_id = 0;
        id = next_id++;
    }
};

// Forward declarations
struct Stmt;
struct FuncDecl;
struct VarDecl;
struct Expr;
struct IntLiteral;
struct CompoundStmt;

// Top-level declarations
struct Program : public Base {
    List<Ptr<FuncDecl>> declarations;

    Program(const List<Ptr<FuncDecl>> &declarations)
        : Base(Kind::Program), declarations(declarations) {}
};

struct FuncDecl : public Base {
    Token returnType;
    Token name;
    List<Ptr<VarDecl>> arguments;
    Ptr<CompoundStmt> body;

    FuncDecl(const Token &returnType, const Token &name,
             const List<Ptr<VarDecl>> &arguments, Ptr<CompoundStmt> body)
        : Base(Kind::FuncDecl), returnType(returnType), name(name),
          arguments(arguments), body(std::move(body)) {}
};

// Statements
struct Stmt : public Base {
    Stmt(Kind kind) : Base(kind) {}
};

struct EmptyStmt : public Stmt {
    EmptyStmt() : Stmt(Kind::EmptyStmt) {}
};

struct IfStmt : public Stmt {
    Ptr<Expr> condition;
    Ptr<Stmt> if_clause;
    Ptr<Stmt> else_clause;

    IfStmt(Ptr<Expr> condition, Ptr<Stmt> if_clause,
           Ptr<Stmt> else_clause = nullptr)
        : Stmt(Kind::IfStmt), condition(std::move(condition)),
          if_clause(std::move(if_clause)), else_clause(std::move(else_clause)) {
    }
};

struct WhileStmt : public Stmt {
    Ptr<Expr> condition;
    Ptr<Stmt> body;

    WhileStmt(Ptr<Expr> condition, Ptr<Stmt> body)
        : Stmt(Kind::WhileStmt), condition(std::move(condition)),
          body(std::move(body)) {}
};

struct ReturnStmt : public Stmt {
    Ptr<Expr> value;

    ReturnStmt(Ptr<Expr> value = nullptr)
        : Stmt(Kind::ReturnStmt), value(std::move(value)) {}
};

struct ExprStmt : public Stmt {
    Ptr<Expr> expr;

    ExprStmt(Ptr<Expr> expr) : Stmt(Kind::ExprStmt), expr(std::move(expr)) {}
};

struct VarDecl : public Stmt {
    Token type;
    Token name;
    Ptr<Expr> init;

    VarDecl(const Token &type, const Token &name, Ptr<Expr> init = nullptr)
        : Stmt(Kind::VarDecl), type(type), name(name), init(std::move(init)) {}
};

struct ArrayDecl : public Stmt {
    Token type;
    Token name;
    Ptr<IntLiteral> size;

    ArrayDecl(const Token &type, const Token &name, Ptr<IntLiteral> size)
        : Stmt(Kind::ArrayDecl), type(type), name(name), size(std::move(size)) {
    }
};

struct CompoundStmt : public Stmt {
    List<Ptr<Stmt>> body;

    CompoundStmt(const List<Ptr<Stmt>> &body)
        : Stmt(Kind::CompoundStmt), body(body) {}
};

// Expressions
struct Expr : public Base {
    Expr(Kind kind) : Base(kind) {}
};

struct BinaryOpExpr : public Expr {
    Ptr<Expr> lhs;
    Token op;
    Ptr<Expr> rhs;

    BinaryOpExpr(Ptr<Expr> lhs, const Token &op, Ptr<Expr> rhs)
        : Expr(Kind::BinaryOpExpr), lhs(std::move(lhs)), op(op),
          rhs(std::move(rhs)) {}
};

struct UnaryOpExpr : public Expr {
    Token op;
    Ptr<Expr> operand;

    UnaryOpExpr(const Token &op, Ptr<Expr> operand)
        : Expr(Kind::UnaryOpExpr), op(op), operand(std::move(operand)) {}
};

struct IntLiteral : public Expr {
    int value;

    IntLiteral(int value) : Expr(Kind::IntLiteral), value(value) {}
};

struct FloatLiteral : public Expr {
    float value;

    FloatLiteral(float value) : Expr(Kind::FloatLiteral), value(value) {}
};

struct StringLiteral : public Expr {
    std::string value;

    StringLiteral(const std::string &value)
        : Expr(Kind::StringLiteral), value(value) {}
};

struct VarRefExpr : public Expr {
    Token name;

    VarRefExpr(const Token &name) : Expr(Kind::VarRefExpr), name(name) {}
};

struct ArrayRefExpr : public Expr {
    Token name;
    Ptr<Expr> index;

    ArrayRefExpr(const Token &name, Ptr<Expr> index)
        : Expr(Kind::ArrayRefExpr), name(name), index(std::move(index)) {}
};

struct FuncCallExpr : public Expr {
    Token name;
    List<Ptr<Expr>> arguments;

    FuncCallExpr(const Token &name, const List<Ptr<Expr>> &arguments)
        : Expr(Kind::FuncCallExpr), name(name), arguments(arguments) {}
};

} // namespace ast

#endif /* end of include guard: AST_HPP */
