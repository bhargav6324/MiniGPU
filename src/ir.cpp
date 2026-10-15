#include "ir.hpp"

#include <sstream>
#include <stdexcept>

namespace minigpu {

namespace {

const char *operationName(IROp operation) {
  switch (operation) {
  case IROp::Constant:
    return "constant";
  case IROp::ThreadIndex:
    return "thread_index";
  case IROp::Add:
    return "add";
  case IROp::Subtract:
    return "subtract";
  case IROp::Multiply:
    return "multiply";
  case IROp::Divide:
    return "divide";
  case IROp::Negate:
    return "negate";
  case IROp::Load:
    return "load";
  case IROp::Store:
    return "store";
  }

  throw std::logic_error("Unknown IR operation");
}

} // namespace

std::string printIR(const IRKernel &kernel) {
  std::ostringstream output;

  output << "kernel " << kernel.name << "(";

  for (std::size_t i = 0; i < kernel.parameters.size(); ++i) {
    if (i != 0) {
      output << ", ";
    }

    const auto &parameter = kernel.parameters[i];

    output << (parameter.type == Type::IntPointer ? "int* " : "int ")
           << parameter.name << "=%" << parameter.value;
  }

  output << ") {\n";

  for (const auto &instruction : kernel.instructions) {
    output << "  ";

    if (instruction.result.has_value()) {
      output << "%" << instruction.result.value() << " = ";
    }

    output << operationName(instruction.operation);

    if (instruction.operation == IROp::Constant) {
      output << " " << instruction.constant;
    } else {
      for (std::size_t i = 0; i < instruction.operands.size(); ++i) {
        output << (i == 0 ? " " : ", ") << "%" << instruction.operands[i];
      }
    }

    output << "\n";
  }

  output << "}\n";

  return output.str();
}

} // namespace minigpu