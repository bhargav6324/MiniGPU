#ifndef MINIGPU_PARSER_HPP
#define MINIGPU_PARSER_HPP

#include <cstddef>
#include <memory>
#include <vector>

#include "ast.hpp"

namespace minigpu {

class Parser {
public:
  explicit Parser(std::vector<Token> tokens);
  Program parse();

private:
  std::vector<Token> tokens_;
  std::size_t position_ = 0;
  const Token &current() const;
  bool match(TokenType type);
  Token expect(TokenType type, const char *description);
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> additive();
  std::unique_ptr<Expr> multiplicative();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();
};

} // namespace minigpu
#endif
