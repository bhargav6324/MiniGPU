#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace minigpu;

void check(bool condition, const std::string &message) {
  if (!condition) throw std::runtime_error(message);
}

Program parse(const std::string &source) {
  return Parser(Lexer(source).tokenize()).parse();
}

void accepted(const std::string &source) {
  auto program = parse(source);
  SemanticAnalyzer().analyze(program);
}

void rejected(const std::string &source, const std::string &fragment) {
  try {
    accepted(source);
  } catch (const std::runtime_error &error) {
    check(std::string(error.what()).find(fragment) != std::string::npos,
          std::string("wrong diagnostic: ") + error.what());
    return;
  }
  throw std::runtime_error("expected rejection: " + source);
}

int main() {
  auto tokens = Lexer("int integer int* a").tokenize();
  check(tokens[0].type == TokenType::Int, "int keyword");
  check(tokens[1].type == TokenType::Identifier, "int prefix identifier");

  const std::string source = R"(
kernel calculate(int* a, int* b, int* output, int n) {
  int offset = i + 1;
  int value = (a[offset * 2] + 5) * (b[i - 1] - a[i] * 2);
  output[offset] = value;
  n = value;
})";
  auto program = parse(source);
  check(program.name == "calculate", "kernel name");
  check(program.parameters.size() == 4, "parameter count");
  check(program.parameters[0].type == Type::IntPointer, "pointer type");
  check(program.parameters[3].type == Type::Int, "scalar type");
  check(program.statements.size() == 4, "multiple statements");
  auto *decl = dynamic_cast<VarDecl *>(program.statements[1].get());
  check(decl != nullptr, "local declaration");
  auto *product = dynamic_cast<BinaryExpr *>(decl->initializer.get());
  check(product && product->token.type == TokenType::Multiply,
        "expression precedence");
  SemanticAnalyzer().analyze(program);

  accepted("kernel empty() {}");
  accepted("kernel k(int* out) { int x; x = -2 + 3 * 4; out[i + x] = x; }");
  accepted("kernel k() { int x = -2147483648; }");
  rejected("kernel k(a) {}", "expected 'int'");
  rejected("kernel k(int* a,) {}", "expected 'int'");
  rejected("kernel k() { int x = ; }", "expected expression");
  rejected("kernel k() { int x = 1; ", "expected '}'");
  rejected("kernel k() {} kernel other() {}", "expected end of file");
  rejected("kernel k(int a, int a) {}", "duplicate");
  rejected("kernel k(int i) {}", "reserved");
  rejected("kernel k() { int x; int x; }", "duplicate");
  rejected("kernel k() { x = 1; }", "unknown identifier");
  rejected("kernel k() { int x; int y = x; }", "uninitialized");
  rejected("kernel k() { int x = x; }", "uninitialized");
  rejected("kernel k(int* a) { int x = a; }", "array requires an index");
  rejected("kernel k(int x) { x[i] = 1; }", "cannot index a scalar");
  rejected("kernel k(int* a) { a = 1; }", "requires an index");
  rejected("kernel k(int* a) { a[a] = 1; }", "array requires an index");
  rejected("kernel k() { i = 1; }", "cannot assign");
  rejected("kernel k() { int x = 2147483648; }", "exceeds int32");

  std::cout << "Phase 2 tests passed\n";
}
