#include "lexer.hpp"

#include <cctype>
#include <stdexcept>
#include <utility>

namespace minigpu {

// Constructor
Lexer::Lexer(std::string source) : source_(std::move(source)) {}

// Check whether we've reached the end of the source
bool Lexer::isAtEnd() const { return position_ >= source_.size(); }

// Look at the current character without consuming it
char Lexer::peek() const {
  if (isAtEnd()) {
    return '\0';
  }

  return source_[position_];
}

// Consume one character and update its position
char Lexer::advance() {
  char current = source_[position_++];

  if (current == '\n') {
    ++line_;
    column_ = 1;
  } else {
    ++column_;
  }

  return current;
}

// Ignore spaces, tabs, and newlines
void Lexer::skipWhitespace() {
  while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek()))) {
    advance();
  }
}

// Recognize identifiers and keywords
Token Lexer::scanIdentifier() {
  const std::size_t start = position_;
  const std::size_t startColumn = column_;
  const std::size_t startLine = line_;

  while (!isAtEnd()) {
    unsigned char c = static_cast<unsigned char>(peek());

    if (std::isalnum(c) || c == '_') {
      advance();
    } else {
      break;
    }
  }

  std::string text = source_.substr(start, position_ - start);

  TokenType type =
      (text == "kernel") ? TokenType::Kernel : TokenType::Identifier;

  return Token{type, text, startLine, startColumn};
}

// Recognize integer literals
Token Lexer::scanNumber() {
  const std::size_t start = position_;
  const std::size_t startColumn = column_;
  const std::size_t startLine = line_;

  while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
    advance();
  }

  std::string text = source_.substr(start, position_ - start);

  return Token{TokenType::Integer, text, startLine, startColumn};
}

// Recognize operators and punctuation
Token Lexer::scanSymbol() {
  const std::size_t startLine = line_;
  const std::size_t startColumn = column_;

  char c = advance();
  TokenType type;

  switch (c) {
  case '+':
    type = TokenType::Plus;
    break;
  case '-':
    type = TokenType::Minus;
    break;
  case '*':
    type = TokenType::Multiply;
    break;
  case '/':
    type = TokenType::Divide;
    break;
  case '=':
    type = TokenType::Assign;
    break;

  case '(':
    type = TokenType::LeftParen;
    break;
  case ')':
    type = TokenType::RightParen;
    break;

  case '{':
    type = TokenType::LeftBrace;
    break;
  case '}':
    type = TokenType::RightBrace;
    break;

  case '[':
    type = TokenType::LeftBracket;
    break;
  case ']':
    type = TokenType::RightBracket;
    break;

  case ',':
    type = TokenType::Comma;
    break;
  case ';':
    type = TokenType::Semicolon;
    break;

  default:
    throw std::runtime_error("Unexpected character '" + std::string(1, c) +
                             "' at line " + std::to_string(startLine) +
                             ", column " + std::to_string(startColumn));
  }

  return Token{type, std::string(1, c), startLine, startColumn};
}

// Convert the entire source code into tokens
std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    skipWhitespace();

    if (isAtEnd()) {
      break;
    }

    unsigned char c = static_cast<unsigned char>(peek());

    if (std::isalpha(c) || c == '_') {
      tokens.push_back(scanIdentifier());
    } else if (std::isdigit(c)) {
      tokens.push_back(scanNumber());
    } else {
      tokens.push_back(scanSymbol());
    }
  }

  tokens.push_back(Token{TokenType::EndOfFile, "", line_, column_});

  return tokens;
}

} // namespace minigpu