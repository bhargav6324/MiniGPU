#include "ir_generator.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace minigpu;

void require(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

IRKernel optimizeExpression(const std::string &expression) {
  const std::string source = "kernel calculate(int* a, int* output, int n) {"
                             "  output[i] = " +
                             expression +
                             ";"
                             "}";

  auto program = Parser(Lexer(source).tokenize()).parse();
  auto kernel = IRGenerator().generate(program);

  foldConstants(kernel);

  return kernel;
}

IRInstruction storedValue(const IRKernel &kernel) {
  const auto &store = kernel.instructions.back();

  require(store.operation == IROp::Store,
          "Optimization must preserve the output store");

  const ValueId value = store.operands.at(2);

  for (const auto &instruction : kernel.instructions) {
    if (instruction.result.has_value() && instruction.result.value() == value) {
      return instruction;
    }
  }

  throw std::runtime_error("Stored value has no instruction");
}

void checkFold(const std::string &expression, std::int32_t expected) {
  const auto kernel = optimizeExpression(expression);
  const auto result = storedValue(kernel);

  require(result.operation == IROp::Constant,
          expression + " should become a constant");

  require(result.constant == expected,
          expression + " produced the wrong value");
}

void checkUnfolded(const std::string &expression, IROp expected) {
  const auto kernel = optimizeExpression(expression);
  const auto result = storedValue(kernel);

  require(result.operation == expected,
          expression + " should remain unchanged");
}

int main() {
  try {
    checkFold("10 * 4", 40);
    checkFold("(2 + 3) * (9 - 5)", 20);
    checkFold("-(8 / 3)", -2);
    checkFold("-7 / 3", -2);

    // These operations must not be evaluated by the optimizer.
    checkUnfolded("5 / 0", IROp::Divide);
    checkUnfolded("2147483647 + 1", IROp::Add);
    checkUnfolded("-2147483648 / -1", IROp::Divide);
    checkUnfolded("-(-2147483648)", IROp::Negate);

    // Runtime inputs are not compile-time constants.
    checkUnfolded("n + 2", IROp::Add);
    checkUnfolded("a[i]", IROp::Load);

    // Repeating the pass should not change an already folded kernel.
    auto kernel = optimizeExpression("(2 + 3) * 4");
    const std::string before = printIR(kernel);

    foldConstants(kernel);

    require(printIR(kernel) == before,
            "Repeated folding should produce the same IR");

    std::cout << "All optimizer tests passed\n";
  } catch (const std::exception &error) {
    std::cerr << "Optimizer test failed: " << error.what() << "\n";
    return 1;
  }

  return 0;
}