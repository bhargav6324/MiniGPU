#include "ir_generator.hpp"
#include "semantic.hpp"

#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace minigpu {
namespace {

class Lowerer {
public:
  IRKernel run(const Program &program) {
    kernel_.name = program.name;

    for (const auto &parameter : program.parameters) {
      const ValueId id = nextValue_++;

      kernel_.parameters.push_back({parameter.name.lexeme, parameter.type, id});

      values_[parameter.name.lexeme] = id;
    }

    for (const auto &statement : program.statements) {
      lowerStatement(*statement);
    }

    return std::move(kernel_);
  }

private:
  IRKernel kernel_;
  ValueId nextValue_ = 0;

  // Maps each variable name to its current IR value.
  std::unordered_map<std::string, ValueId> values_;

  ValueId emit(IROp operation, std::vector<ValueId> operands,
               const Token &source, std::int32_t constant = 0) {
    const ValueId result = nextValue_++;

    kernel_.instructions.push_back(
        {operation, result, std::move(operands), constant, source});

    return result;
  }

  ValueId lowerExpression(const Expr &expression) {
    if (const auto *literal = dynamic_cast<const IntegerExpr *>(&expression)) {
      const auto value =
          static_cast<std::int32_t>(std::stoll(literal->token.lexeme));

      return emit(IROp::Constant, {}, literal->token, value);
    }

    if (const auto *name = dynamic_cast<const NameExpr *>(&expression)) {
      if (name->token.lexeme == "i" && values_.find("i") == values_.end()) {
        values_["i"] = emit(IROp::ThreadIndex, {}, name->token);
      }

      return values_.at(name->token.lexeme);
    }

    if (const auto *indexed = dynamic_cast<const IndexExpr *>(&expression)) {
      const ValueId array = values_.at(indexed->token.lexeme);
      const ValueId index = lowerExpression(*indexed->index);

      return emit(IROp::Load, {array, index}, indexed->token);
    }

    if (const auto *unary = dynamic_cast<const UnaryExpr *>(&expression)) {
      // Handle the signed 32-bit minimum directly.
      if (unary->token.type == TokenType::Minus) {
        const auto *literal =
            dynamic_cast<const IntegerExpr *>(unary->operand.get());

        if (literal && literal->token.lexeme == "2147483648") {
          return emit(IROp::Constant, {}, unary->token,
                      std::numeric_limits<std::int32_t>::min());
        }
      }

      const ValueId operand = lowerExpression(*unary->operand);

      if (unary->token.type == TokenType::Plus) {
        return operand;
      }

      return emit(IROp::Negate, {operand}, unary->token);
    }

    if (const auto *binary = dynamic_cast<const BinaryExpr *>(&expression)) {
      const ValueId left = lowerExpression(*binary->left);
      const ValueId right = lowerExpression(*binary->right);

      IROp operation;

      switch (binary->token.type) {
      case TokenType::Plus:
        operation = IROp::Add;
        break;
      case TokenType::Minus:
        operation = IROp::Subtract;
        break;
      case TokenType::Multiply:
        operation = IROp::Multiply;
        break;
      case TokenType::Divide:
        operation = IROp::Divide;
        break;
      default:
        throw std::logic_error("Unsupported binary operator");
      }

      return emit(operation, {left, right}, binary->token);
    }

    throw std::logic_error("Unsupported AST expression");
  }

  void lowerStatement(const Stmt &statement) {
    if (const auto *declaration = dynamic_cast<const VarDecl *>(&statement)) {
      if (declaration->initializer) {
        const ValueId value = lowerExpression(*declaration->initializer);

        values_[declaration->token.lexeme] = value;
      }

      return;
    }

    if (const auto *assignment = dynamic_cast<const Assignment *>(&statement)) {
      if (assignment->index) {
        const ValueId array = values_.at(assignment->token.lexeme);
        const ValueId index = lowerExpression(*assignment->index);
        const ValueId value = lowerExpression(*assignment->value);

        kernel_.instructions.push_back({IROp::Store,
                                        std::nullopt,
                                        {array, index, value},
                                        0,
                                        assignment->token});
      } else {
        const ValueId value = lowerExpression(*assignment->value);

        values_[assignment->token.lexeme] = value;
      }

      return;
    }

    throw std::logic_error("Unsupported AST statement");
  }
};

} // namespace

IRKernel IRGenerator::generate(const Program &program) const {
  SemanticAnalyzer().analyze(program);
  return Lowerer().run(program);
}

} // namespace minigpu