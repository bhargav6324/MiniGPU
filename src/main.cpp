#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

using namespace minigpu;

// Convert token types into readable names
std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::Kernel:
    return "KEYWORD_KERNEL";
  case TokenType::Identifier:
    return "IDENTIFIER";
  case TokenType::Integer:
    return "INTEGER";

  case TokenType::Plus:
    return "PLUS";
  case TokenType::Minus:
    return "MINUS";
  case TokenType::Multiply:
    return "MULTIPLY";
  case TokenType::Divide:
    return "DIVIDE";
  case TokenType::Assign:
    return "ASSIGN";

  case TokenType::LeftParen:
    return "LEFT_PAREN";
  case TokenType::RightParen:
    return "RIGHT_PAREN";
  case TokenType::LeftBrace:
    return "LEFT_BRACE";
  case TokenType::RightBrace:
    return "RIGHT_BRACE";
  case TokenType::LeftBracket:
    return "LEFT_BRACKET";
  case TokenType::RightBracket:
    return "RIGHT_BRACKET";

  case TokenType::Comma:
    return "COMMA";
  case TokenType::Semicolon:
    return "SEMICOLON";
  case TokenType::EndOfFile:
    return "END_OF_FILE";
  }

  return "UNKNOWN";
}

int main(int argc, char *argv[]) {
  if (argc != 3 || std::string(argv[1]) != "--tokens") {
    std::cerr << "Usage: minigpu --tokens <source.mgpu>\n";
    return 1;
  }

  try {
    // Open the source file
    std::ifstream file(argv[2]);

    if (!file.is_open()) {
      throw std::runtime_error("Cannot open source file: " +
                               std::string(argv[2]));
    }

    // Read the entire file
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    if (file.bad()) {
      throw std::runtime_error("Error reading source file");
    }

    // Create lexer and tokenize source
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // Print tokens
    std::cout << "TOKEN TYPE\tLEXEME\tLINE\tCOLUMN\n";
    std::cout << "----------------------------------------\n";

    for (const Token &token : tokens) {
      std::cout << tokenTypeToString(token.type) << '\t' << token.lexeme << '\t'
                << token.line << '\t' << token.column << '\n';
    }

  } catch (const std::exception &error) {
    std::cerr << "MiniGPU Error: " << error.what() << '\n';
    return 1;
  }

  return 0;
}