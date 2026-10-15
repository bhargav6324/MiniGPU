#ifndef MINIGPU_TOKEN_HPP
#define MINIGPU_TOKEN_HPP

#include <cstddef>
#include <string>

namespace minigpu {

// categories of tokens supported by MiniGPU.
enum class TokenType {
  // Keywords
  Kernel,
  Int,

  // Identifiers and literals
  Identifier,
  Integer,

  // Arithmetic operators
  Plus,
  Minus,
  Multiply,
  Divide,

  // Assignment
  Assign,

  // Delimiters
  LeftParen,
  RightParen,
  LeftBrace,
  RightBrace,
  LeftBracket,
  RightBracket,

  // Punctuation
  Comma,
  Semicolon,

  // Special token
  EndOfFile
};

struct Token {
  TokenType type;
  std::string lexeme;
  std::size_t line;
  std::size_t column;
};

} // namespace minigpu

#endif // MINIGPU_TOKEN_HPP