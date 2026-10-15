#include "lexer.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace minigpu;

// Test 1: Basic arithmetic expression
void testBasicTokens() {
  Lexer lexer("a + b");

  std::vector<Token> tokens = lexer.tokenize();

  assert(tokens.size() == 4);

  assert(tokens[0].type == TokenType::Identifier);
  assert(tokens[0].lexeme == "a");

  assert(tokens[1].type == TokenType::Plus);

  assert(tokens[2].type == TokenType::Identifier);
  assert(tokens[2].lexeme == "b");

  assert(tokens[3].type == TokenType::EndOfFile);

  std::cout << "[PASS] Basic tokenization\n";
}

// Test 2: Arithmetic operators
void testOperators() {
  Lexer lexer("+ - * / =");

  auto tokens = lexer.tokenize();

  assert(tokens[0].type == TokenType::Plus);
  assert(tokens[1].type == TokenType::Minus);
  assert(tokens[2].type == TokenType::Multiply);
  assert(tokens[3].type == TokenType::Divide);
  assert(tokens[4].type == TokenType::Assign);
  assert(tokens[5].type == TokenType::EndOfFile);

  std::cout << "[PASS] Arithmetic operators\n";
}

// Test 3: Keywords and identifiers
void testKeywords() {
  Lexer lexer("kernel int calculate integer");

  auto tokens = lexer.tokenize();

  assert(tokens[0].type == TokenType::Kernel);
  assert(tokens[1].type == TokenType::Int);
  assert(tokens[2].type == TokenType::Identifier);
  assert(tokens[3].type == TokenType::Identifier);

  std::cout << "[PASS] Keywords and identifiers\n";
}

// Test 4: Integer literals
void testIntegers() {
  Lexer lexer("10 25 100");

  auto tokens = lexer.tokenize();

  assert(tokens[0].lexeme == "10");
  assert(tokens[1].lexeme == "25");
  assert(tokens[2].lexeme == "100");

  std::cout << "[PASS] Integer literals\n";
}

// Test 5: Line and column tracking
void testPositions() {
  Lexer lexer("a\n  b");

  auto tokens = lexer.tokenize();

  assert(tokens[0].line == 1);
  assert(tokens[0].column == 1);

  assert(tokens[1].line == 2);
  assert(tokens[1].column == 3);

  std::cout << "[PASS] Source positions\n";
}

// Test 6: Invalid characters
void testInvalidCharacter() {
  bool caught = false;

  try {
    Lexer lexer("a @ b");
    lexer.tokenize();
  } catch (const std::runtime_error &) {
    caught = true;
  }

  assert(caught);

  std::cout << "[PASS] Invalid character detection\n";
}

// Test 7: Empty input
void testEmptyInput() {
  Lexer lexer("");

  auto tokens = lexer.tokenize();

  assert(tokens.size() == 1);
  assert(tokens[0].type == TokenType::EndOfFile);

  std::cout << "[PASS] Empty input\n";
}

int main() {
  testBasicTokens();
  testOperators();
  testKeywords();
  testIntegers();
  testPositions();
  testInvalidCharacter();
  testEmptyInput();

  std::cout << "\nAll lexer tests passed!\n";

  return 0;
}
