#include "ir_generator.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

using namespace minigpu;

void require(bool condition, const std::string &message) {
  if (!condition)
    throw std::runtime_error(message);
}

IRKernel optimize(const std::string &source) {
  auto program = Parser(Lexer(source).tokenize()).parse();
  auto kernel = IRGenerator().generate(program);

  foldConstants(kernel);
  eliminateDeadCode(kernel);

  return kernel;
}

void checkDependencies(const IRKernel &kernel) {
  std::unordered_set<ValueId> defined;

  for (const auto &parameter : kernel.parameters) {
    defined.insert(parameter.value);
  }

  for (const auto &instruction : kernel.instructions) {
    for (ValueId operand : instruction.operands) {
      require(defined.count(operand) != 0,
              "An instruction uses a missing value");
    }

    if (instruction.result.has_value()) {
      require(defined.insert(instruction.result.value()).second,
              "A value ID is defined twice");
    }
  }
}

int main() {
  try {
    auto kernel = optimize("kernel calculate(int* a, int* output) {"
                           "  output[i] = a[i] + (10 * 4);"
                           "}");

    require(kernel.instructions.size() == 5,
            "Unused constants should be removed");

    require(kernel.instructions.at(2).operation == IROp::Constant &&
                kernel.instructions.at(2).constant == 40,
            "The folded constant must remain");

    checkDependencies(kernel);

    const std::string before = printIR(kernel);
    eliminateDeadCode(kernel);

    require(printIR(kernel) == before,
            "Repeating DCE should not change the result");

    kernel = optimize("kernel calculate(int n) {"
                      "  int x = n + 1;"
                      "  int y = x * 2;"
                      "}");

    require(kernel.instructions.empty(),
            "The entire unused calculation chain should disappear");

    kernel = optimize("kernel calculate(int* a, int* output, int n) {"
                      "  output[i] = 1;"
                      "  int unused = a[i];"
                      "  int quotient = n / 0;"
                      "  output[i] = 2;"
                      "}");

    std::vector<IROp> requiredOperations;

    for (const auto &instruction : kernel.instructions) {
      if (instruction.operation == IROp::Store ||
          instruction.operation == IROp::Load ||
          instruction.operation == IROp::Divide) {
        requiredOperations.push_back(instruction.operation);
      }
    }

    const std::vector<IROp> expected = {IROp::Store, IROp::Load, IROp::Divide,
                                        IROp::Store};

    require(requiredOperations == expected,
            "Required operations must remain in source order");

    checkDependencies(kernel);

    std::cout << "All DCE tests passed\n";
  } catch (const std::exception &error) {
    std::cerr << "DCE test failed: " << error.what() << "\n";
    return 1;
  }

  return 0;
}