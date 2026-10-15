#include "parser.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace minigpu {

namespace {
std::runtime_error syntaxError(const Token &token, const std::string &message) {
  return std::runtime_error("Syntax error at line " +
                            std::to_string(token.line) + ", column " +
                            std::to_string(token.column) + ": " + message);
}
} // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {
  if (tokens_.empty() || tokens_.back().type != TokenType::EndOfFile) {
    throw std::invalid_argument("Parser requires an EOF terminated token list");
  }
}

const Token &Parser::current() const { return tokens_[position_]; }

bool Parser::match(TokenType type) {
  if (current().type != type) return false;
  ++position_;
  return true;
}

Token Parser::expect(TokenType type, const char *description) {
  if (current().type != type)
    throw syntaxError(current(), std::string("expected ") + description);
  return tokens_[position_++];
}

Program Parser::parse() {
  expect(TokenType::Kernel, "'kernel'");
  Program program;
  program.name = expect(TokenType::Identifier, "kernel name").lexeme;
  expect(TokenType::LeftParen, "'('");
  if (current().type != TokenType::RightParen) {
    do {
      expect(TokenType::Int, "'int' parameter type");
      const Type type = match(TokenType::Multiply) ? Type::IntPointer : Type::Int;
      program.parameters.push_back(
          Parameter{type, expect(TokenType::Identifier, "parameter name")});
    } while (match(TokenType::Comma));
  }
  expect(TokenType::RightParen, "')'");
  expect(TokenType::LeftBrace, "'{'");
  while (current().type != TokenType::RightBrace) {
    if (current().type == TokenType::EndOfFile)
      throw syntaxError(current(), "expected '}'");
    program.statements.push_back(statement());
  }
  expect(TokenType::RightBrace, "'}'");
  expect(TokenType::EndOfFile, "end of file");
  return program;
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match(TokenType::Int)) {
    Token name = expect(TokenType::Identifier, "local variable name");
    std::unique_ptr<Expr> initializer;
    if (match(TokenType::Assign)) initializer = expression();
    expect(TokenType::Semicolon, "';'");
    return std::make_unique<VarDecl>(std::move(name), std::move(initializer));
  }

  Token name = expect(TokenType::Identifier, "assignment target");
  std::unique_ptr<Expr> index;
  if (match(TokenType::LeftBracket)) {
    index = expression();
    expect(TokenType::RightBracket, "']'");
  }
  expect(TokenType::Assign, "'='");
  auto value = expression();
  expect(TokenType::Semicolon, "';'");
  return std::make_unique<Assignment>(std::move(name), std::move(index),
                                      std::move(value));
}

std::unique_ptr<Expr> Parser::expression() { return additive(); }

std::unique_ptr<Expr> Parser::additive() {
  auto left = multiplicative();
  while (current().type == TokenType::Plus ||
         current().type == TokenType::Minus) {
    Token op = tokens_[position_++];
    left = std::make_unique<BinaryExpr>(std::move(op), std::move(left),
                                        multiplicative());
  }
  return left;
}

std::unique_ptr<Expr> Parser::multiplicative() {
  auto left = unary();
  while (current().type == TokenType::Multiply ||
         current().type == TokenType::Divide) {
    Token op = tokens_[position_++];
    left = std::make_unique<BinaryExpr>(std::move(op), std::move(left), unary());
  }
  return left;
}

std::unique_ptr<Expr> Parser::unary() {
  if (current().type == TokenType::Plus ||
      current().type == TokenType::Minus) {
    Token op = tokens_[position_++];
    return std::make_unique<UnaryExpr>(std::move(op), unary());
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (current().type == TokenType::Integer)
    return std::make_unique<IntegerExpr>(tokens_[position_++]);
  if (current().type == TokenType::Identifier) {
    Token name = tokens_[position_++];
    if (match(TokenType::LeftBracket)) {
      auto index = expression();
      expect(TokenType::RightBracket, "']'");
      return std::make_unique<IndexExpr>(std::move(name), std::move(index));
    }
    return std::make_unique<NameExpr>(std::move(name));
  }
  if (match(TokenType::LeftParen)) {
    auto expr = expression();
    expect(TokenType::RightParen, "')'");
    return expr;
  }
  throw syntaxError(current(), "expected expression");
}

} // namespace minigpu
