#ifndef MINIGPU_LEXER_HPP
#define MINIGPU_LEXER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "token.hpp"

namespace minigpu {

class Lexer {
public:
  explicit Lexer(std::string source);

  std::vector<Token> tokenize();

private:
  std::string source_;

  std::size_t position_ = 0;
  std::size_t line_ = 1;
  std::size_t column_ = 1;

  char peek() const;
  char advance();
  bool isAtEnd() const;

  void skipWhitespace();

  Token scanIdentifier();
  Token scanNumber();
  Token scanSymbol();
};

} // namespace minigpu

#endif // MINIGPU_LEXER_HPP