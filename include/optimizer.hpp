#ifndef MINIGPU_OPTIMIZER_HPP
#define MINIGPU_OPTIMIZER_HPP

#include "ir.hpp"

namespace minigpu {

void foldConstants(IRKernel &kernel);
void eliminateDeadCode(IRKernel &kernel);

} // namespace minigpu

#endif // MINIGPU_OPTIMIZER_HPP