#include "optimizer.hpp"

#include <algorithm>
#include <limits>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace minigpu {
namespace {

std::optional<std::int64_t> evaluate(IROp operation, std::int64_t left,
                                     std::int64_t right) {
  switch (operation) {
  case IROp::Add:
    return left + right;

  case IROp::Subtract:
    return left - right;

  case IROp::Multiply:
    return left * right;

  case IROp::Divide:
    if (right == 0)
      return std::nullopt;
    return left / right;

  case IROp::Negate:
    return -left;

  default:
    return std::nullopt;
  }
}

} // namespace

void foldConstants(IRKernel &kernel) {
  std::unordered_map<ValueId, std::int32_t> constants;

  for (auto &instruction : kernel.instructions) {
    if (!instruction.result.has_value())
      continue;

    const ValueId id = instruction.result.value();

    if (instruction.operation == IROp::Constant) {
      constants[id] = instruction.constant;
      continue;
    }

    const bool unary = instruction.operation == IROp::Negate;

    const bool binary = instruction.operation == IROp::Add ||
                        instruction.operation == IROp::Subtract ||
                        instruction.operation == IROp::Multiply ||
                        instruction.operation == IROp::Divide;

    if (!unary && !binary)
      continue;

    const auto left = constants.find(instruction.operands.at(0));

    if (left == constants.end())
      continue;

    std::int64_t right = 0;

    if (binary) {
      const auto found = constants.find(instruction.operands.at(1));

      if (found == constants.end())
        continue;

      right = found->second;
    }

    const auto result = evaluate(instruction.operation, left->second, right);

    if (!result.has_value())
      continue;

    if (result.value() < std::numeric_limits<std::int32_t>::min() ||
        result.value() > std::numeric_limits<std::int32_t>::max()) {
      continue;
    }

    instruction.operation = IROp::Constant;
    instruction.constant = static_cast<std::int32_t>(result.value());
    instruction.operands.clear();

    constants[id] = instruction.constant;
  }
}

void eliminateDeadCode(IRKernel &kernel) {
  std::unordered_set<ValueId> live;
  std::vector<IRInstruction> kept;

  for (auto it = kernel.instructions.rbegin(); it != kernel.instructions.rend();
       ++it) {
    const auto &instruction = *it;

    const bool resultUsed = instruction.result.has_value() &&
                            live.count(instruction.result.value()) != 0;

    // Preserve memory operations and potentially invalid divisions.
    const bool mustKeep = !instruction.result.has_value() ||
                          instruction.operation == IROp::Load ||
                          instruction.operation == IROp::Divide;

    if (!mustKeep && !resultUsed)
      continue;

    kept.push_back(instruction);

    if (instruction.result.has_value()) {
      live.erase(instruction.result.value());
    }

    for (ValueId operand : instruction.operands) {
      live.insert(operand);
    }
  }

  std::reverse(kept.begin(), kept.end());
  kernel.instructions = std::move(kept);
}

} // namespace minigpu