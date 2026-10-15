#include "ir_generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace minigpu;

void require(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

IRKernel compileToIR(const std::string &source) {
  auto program = Parser(Lexer(source).tokenize()).parse();
  return IRGenerator().generate(program);
}

void testBasicIR() {
  auto kernel = compileToIR("kernel calculate(int* a, int* output) {"
                            "  output[i] = a[i] + 2;"
                            "}");

  const std::string expected = "kernel calculate(int* a=%0, int* output=%1) {\n"
                               "  %2 = thread_index\n"
                               "  %3 = load %0, %2\n"
                               "  %4 = constant 2\n"
                               "  %5 = add %3, %4\n"
                               "  store %1, %2, %5\n"
                               "}\n";

  require(printIR(kernel) == expected, "Basic IR output is incorrect");
}

void testScalarReassignment() {
  auto kernel = compileToIR("kernel calculate(int* output, int n) {"
                            "  int previous = n;"
                            "  n = n + 1;"
                            "  output[0] = previous;"
                            "  output[1] = n;"
                            "}");

  std::vector<const IRInstruction *> stores;
  std::optional<ValueId> updatedValue;

  for (const auto &instruction : kernel.instructions) {
    if (instruction.operation == IROp::Add) {
      updatedValue = instruction.result;
    }

    if (instruction.operation == IROp::Store) {
      stores.push_back(&instruction);
    }
  }

  require(stores.size() == 2, "Expected two stores");
  require(updatedValue.has_value(), "Expected an addition result");

  require(stores[0]->operands.at(2) == kernel.parameters.at(1).value,
          "previous should retain the original n value");

  require(stores[1]->operands.at(2) == updatedValue.value(),
          "n should refer to the updated value");
}

void testMemoryOrder() {
  auto kernel = compileToIR("kernel calculate(int* output) {"
                            "  output[0] = 7;"
                            "  int value = output[0];"
                            "  output[1] = value;"
                            "}");

  std::vector<IROp> memoryOperations;

  for (const auto &instruction : kernel.instructions) {
    if (instruction.operation == IROp::Load ||
        instruction.operation == IROp::Store) {
      memoryOperations.push_back(instruction.operation);
    }
  }

  const std::vector<IROp> expected = {IROp::Store, IROp::Load, IROp::Store};

  require(memoryOperations == expected,
          "Memory operations must preserve source order");
}

void testSignedMinimum() {
  auto kernel = compileToIR("kernel calculate(int* output) {"
                            "  output[0] = -2147483648;"
                            "}");

  bool found = false;

  for (const auto &instruction : kernel.instructions) {
    if (instruction.operation == IROp::Constant &&
        instruction.constant == std::numeric_limits<std::int32_t>::min()) {
      found = true;
    }
  }

  require(found, "Signed minimum should become an IR constant");
}

int main() {
  try {
    testBasicIR();
    testScalarReassignment();
    testMemoryOrder();
    testSignedMinimum();

    std::cout << "All IR tests passed\n";
  } catch (const std::exception &error) {
    std::cerr << "IR test failed: " << error.what() << "\n";
    return 1;
  }

  return 0;
}