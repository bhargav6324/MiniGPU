#ifndef MINIGPU_IR_HPP
#define MINIGPU_IR_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "ast.hpp"

namespace minigpu {

// Every parameter or computed value receives a unique ID.
using ValueId = std::size_t;

// Operations supported by our intermediate representation.
enum class IROp {
  Constant,
  ThreadIndex,

  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,

  Load,
  Store
};

struct IRParameter {
  std::string name;
  Type type;
  ValueId value;
};

struct IRInstruction {
  IROp operation;

  // Store has no result. Other operations produce a value.
  std::optional<ValueId> result;

  // IDs of the values used by this instruction.
  //
  // Constant, ThreadIndex: no operands
  // Negate:                value
  // Arithmetic:            left, right
  // Load:                  array, index
  // Store:                 array, index, value
  std::vector<ValueId> operands;

  // Used only by Constant.
  std::int32_t constant = 0;

  // Original source position for diagnostics.
  Token source;
};

struct IRKernel {
  std::string name;
  std::vector<IRParameter> parameters;
  std::vector<IRInstruction> instructions;
};
std::string printIR(const IRKernel &kernel);

} // namespace minigpu

#endif // MINIGPU_IR_HPP