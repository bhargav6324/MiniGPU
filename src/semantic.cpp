#include "semantic.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace minigpu {
namespace {

struct Symbol {
  Type type;
  bool initialized;
  bool writable;
};
using Symbols = std::unordered_map<std::string, Symbol>;

std::runtime_error semanticError(const Token &token, const std::string &message) {
  return std::runtime_error("Semantic error at line " +
                            std::to_string(token.line) + ", column " +
                            std::to_string(token.column) + ": " + message);
}

Symbol &lookup(Symbols &symbols, const Token &token) {
  auto found = symbols.find(token.lexeme);
  if (found == symbols.end())
    throw semanticError(token, "unknown identifier '" + token.lexeme + "'");
  return found->second;
}

void requireInt(const Expr &expr, Symbols &symbols) {
  if (const auto *literal = dynamic_cast<const IntegerExpr *>(&expr)) {
    try {
      const auto value = std::stoll(literal->token.lexeme);
      if (value > std::numeric_limits<std::int32_t>::max())
        throw semanticError(literal->token, "integer literal exceeds int32");
    } catch (const std::out_of_range &) {
      throw semanticError(literal->token, "integer literal exceeds int32");
    }
    return;
  }
  if (const auto *name = dynamic_cast<const NameExpr *>(&expr)) {
    auto &symbol = lookup(symbols, name->token);
    if (symbol.type != Type::Int)
      throw semanticError(name->token, "array requires an index");
    if (!symbol.initialized)
      throw semanticError(name->token, "use of uninitialized variable");
    return;
  }
  if (const auto *indexed = dynamic_cast<const IndexExpr *>(&expr)) {
    auto &symbol = lookup(symbols, indexed->token);
    if (symbol.type != Type::IntPointer)
      throw semanticError(indexed->token, "cannot index a scalar");
    requireInt(*indexed->index, symbols);
    return;
  }
  if (const auto *unary = dynamic_cast<const UnaryExpr *>(&expr)) {
    if (unary->token.type == TokenType::Minus) {
      if (const auto *literal =
              dynamic_cast<const IntegerExpr *>(unary->operand.get())) {
        if (literal->token.lexeme == "2147483648") return;
      }
    }
    requireInt(*unary->operand, symbols);
    return;
  }
  if (const auto *binary = dynamic_cast<const BinaryExpr *>(&expr)) {
    requireInt(*binary->left, symbols);
    requireInt(*binary->right, symbols);
    return;
  }
  throw semanticError(expr.token, "unsupported expression");
}

} // namespace

void SemanticAnalyzer::analyze(const Program &program) const {
  Symbols symbols;
  // Thread index is a read-only predefined scalar in this phase.
  symbols.emplace("i", Symbol{Type::Int, true, false});
  for (const auto &parameter : program.parameters) {
    if (!symbols.emplace(parameter.name.lexeme,
                         Symbol{parameter.type, true, true}).second)
      throw semanticError(parameter.name,
                          "duplicate or reserved identifier '" +
                              parameter.name.lexeme + "'");
  }

  for (const auto &statement : program.statements) {
    if (const auto *decl = dynamic_cast<const VarDecl *>(statement.get())) {
      if (!symbols.emplace(decl->token.lexeme,
                           Symbol{Type::Int, false, true}).second)
        throw semanticError(decl->token,
                            "duplicate or reserved identifier '" +
                                decl->token.lexeme + "'");
      if (decl->initializer) {
        requireInt(*decl->initializer, symbols);
        symbols.at(decl->token.lexeme).initialized = true;
      }
      continue;
    }
    const auto *assign = dynamic_cast<const Assignment *>(statement.get());
    if (!assign)
      throw semanticError(statement->token, "unsupported statement");
    auto &target = lookup(symbols, assign->token);
    if (!target.writable)
      throw semanticError(assign->token, "cannot assign to built-in index");
    if (assign->index) {
      if (target.type != Type::IntPointer)
        throw semanticError(assign->token, "cannot index a scalar");
      requireInt(*assign->index, symbols);
    } else if (target.type != Type::Int) {
      throw semanticError(assign->token, "array assignment requires an index");
    }
    requireInt(*assign->value, symbols);
    if (!assign->index) target.initialized = true;
  }
}

} // namespace minigpu
