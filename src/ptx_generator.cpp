#include "ptx_generator.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>

namespace minigpu {
namespace {

std::string r(ValueId id) { return "%r" + std::to_string(id); }

std::string rd(ValueId id) { return "%rd" + std::to_string(id); }

} // namespace

std::string
PTXGenerator::generate(const IRKernel &kernel,
                       const std::string &elementCountParameter) const {
  std::optional<ValueId> count;

  for (const auto &parameter : kernel.parameters) {
    if (parameter.name == elementCountParameter) {
      if (parameter.type != Type::Int) {
        throw std::runtime_error("Element count parameter must be int");
      }

      count = parameter.value;
    }
  }

  if (!count.has_value()) {
    throw std::runtime_error("Unknown element count parameter: " +
                             elementCountParameter);
  }

  std::ostringstream out;

  out << ".version 7.0\n.target sm_75\n.address_size 64\n\n";
  out << ".visible .entry mgpu_" << kernel.name << "(\n";

  for (std::size_t i = 0; i < kernel.parameters.size(); ++i) {
    const auto &parameter = kernel.parameters[i];

    out << "  .param "
        << (parameter.type == Type::IntPointer ? ".u64 " : ".s32 ") << "p"
        << parameter.value;

    out << (i + 1 == kernel.parameters.size() ? "\n" : ",\n");
  }

  out << ") {\n"
      << "  .reg .b32 %thread, %block, %block_size, %index;\n"
      << "  .reg .b64 %offset, %address;\n"
      << "  .reg .pred %exit;\n";

  for (const auto &parameter : kernel.parameters) {
    const bool pointer = parameter.type == Type::IntPointer;

    out << "  .reg " << (pointer ? ".b64 " : ".b32 ")
        << (pointer ? rd(parameter.value) : r(parameter.value)) << ";\n";
  }

  for (const auto &instruction : kernel.instructions) {
    if (instruction.result.has_value()) {
      out << "  .reg .b32 " << r(instruction.result.value()) << ";\n";
    }
  }

  out << "\n";

  for (const auto &parameter : kernel.parameters) {
    const bool pointer = parameter.type == Type::IntPointer;

    const std::string reg = pointer ? rd(parameter.value) : r(parameter.value);

    out << "  ld.param." << (pointer ? "u64 " : "s32 ") << reg << ", [p"
        << parameter.value << "];\n";

    if (pointer) {
      out << "  cvta.to.global.u64 " << reg << ", " << reg << ";\n";
    }
  }

  out << "  mov.u32 %thread, %tid.x;\n"
      << "  mov.u32 %block, %ctaid.x;\n"
      << "  mov.u32 %block_size, %ntid.x;\n"
      << "  mad.lo.u32 %index, %block, %block_size, %thread;\n"
      << "  setp.le.s32 %exit, " << r(count.value()) << ", 0;\n"
      << "  @%exit bra MGPU_DONE;\n"
      << "  setp.ge.u32 %exit, %index, " << r(count.value()) << ";\n"
      << "  @%exit bra MGPU_DONE;\n\n";

  for (const auto &instruction : kernel.instructions) {
    auto destination = [&]() { return r(instruction.result.value()); };

    auto operand = [&](std::size_t i) { return r(instruction.operands.at(i)); };

    auto binary = [&](const char *operation) {
      out << "  " << operation << " " << destination() << ", " << operand(0)
          << ", " << operand(1) << ";\n";
    };

    switch (instruction.operation) {
    case IROp::Constant:
      out << "  mov.s32 " << destination() << ", " << instruction.constant
          << ";\n";
      break;

    case IROp::ThreadIndex:
      out << "  mov.u32 " << destination() << ", %index;\n";
      break;

    case IROp::Add:
      binary("add.s32");
      break;

    case IROp::Subtract:
      binary("sub.s32");
      break;

    case IROp::Multiply:
      binary("mul.lo.s32");
      break;

    case IROp::Divide:
      binary("div.s32");
      break;

    case IROp::Negate:
      out << "  neg.s32 " << destination() << ", " << operand(0) << ";\n";
      break;

    case IROp::Load:
    case IROp::Store:
      out << "  mul.wide.s32 %offset, " << operand(1) << ", 4;\n"
          << "  add.u64 %address, " << rd(instruction.operands.at(0))
          << ", %offset;\n";

      if (instruction.operation == IROp::Load) {
        out << "  ld.global.s32 " << destination() << ", [%address];\n";
      } else {
        out << "  st.global.s32 [%address], " << operand(2) << ";\n";
      }
      break;

    default:
      throw std::runtime_error("Unsupported IR operation for PTX");
    }
  }

  out << "\nMGPU_DONE:\n  ret;\n}\n";

  return out.str();
}

} // namespace minigpu