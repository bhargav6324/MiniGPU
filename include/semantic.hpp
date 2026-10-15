#ifndef MINIGPU_SEMANTIC_HPP
#define MINIGPU_SEMANTIC_HPP

#include "ast.hpp"

namespace minigpu {

class SemanticAnalyzer {
public:
  void analyze(const Program &program) const;
};

} // namespace minigpu
#endif
