#ifndef MINIGPU_PTX_GENERATOR_HPP
#define MINIGPU_PTX_GENERATOR_HPP

#include <string>

#include "ir.hpp"

namespace minigpu {

class PTXGenerator {
public:
  // Convert IR into NVIDIA PTX assembly.
  //
  // elementCountParameter must name an int parameter in the kernel.
  std::string generate(const IRKernel &kernel,
                       const std::string &elementCountParameter) const;
};

} // namespace minigpu

#endif // MINIGPU_PTX_GENERATOR_HPP