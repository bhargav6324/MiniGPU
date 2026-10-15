#ifndef MINIGPU_AST_HPP
#define MINIGPU_AST_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "token.hpp"

namespace minigpu {

enum class Type { Int, IntPointer };

struct Expr {
  explicit Expr(Token token) : token(std::move(token)) {}
  virtual ~Expr() = default;
  Token token;
};

struct IntegerExpr final : Expr {
  explicit IntegerExpr(Token token) : Expr(std::move(token)) {}
};

struct NameExpr final : Expr {
  explicit NameExpr(Token token) : Expr(std::move(token)) {}
};

struct IndexExpr final : Expr {
  IndexExpr(Token token, std::unique_ptr<Expr> index)
      : Expr(std::move(token)), index(std::move(index)) {}
  std::unique_ptr<Expr> index;
};

struct UnaryExpr final : Expr {
  UnaryExpr(Token op, std::unique_ptr<Expr> operand)
      : Expr(std::move(op)), operand(std::move(operand)) {}
  std::unique_ptr<Expr> operand;
};

struct BinaryExpr final : Expr {
  BinaryExpr(Token op, std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> right)
      : Expr(std::move(op)), left(std::move(left)), right(std::move(right)) {}
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
};

struct Stmt {
  explicit Stmt(Token token) : token(std::move(token)) {}
  virtual ~Stmt() = default;
  Token token;
};

struct VarDecl final : Stmt {
  VarDecl(Token name, std::unique_ptr<Expr> initializer)
      : Stmt(std::move(name)), initializer(std::move(initializer)) {}
  std::unique_ptr<Expr> initializer;
};

struct Assignment final : Stmt {
  Assignment(Token name, std::unique_ptr<Expr> index,
             std::unique_ptr<Expr> value)
      : Stmt(std::move(name)), index(std::move(index)),
        value(std::move(value)) {}
  std::unique_ptr<Expr> index; // null for a scalar assignment
  std::unique_ptr<Expr> value;
};

struct Parameter {
  Type type;
  Token name;
};

struct Program {
  std::string name;
  std::vector<Parameter> parameters;
  std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace minigpu
#endif
