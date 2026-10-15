#ifndef MINIGPU_IR_GENERATOR_HPP
#define MINIGPU_IR_GENERATOR_HPP

#include "ast.hpp"
#include "ir.hpp"

namespace minigpu {

class IRGenerator {
public:
  // Convert a semantically checked AST into IR instructions.
  IRKernel generate(const Program &program) const;
};

} // namespace minigpu

#endif // MINIGPU_IR_GENERATOR_HPP