#include "ir_generator.hpp"
#include "lexer.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "ptx_generator.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace minigpu;

void require(bool condition, const std::string &message) {
  if (!condition)
    throw std::runtime_error(message);
}

std::string generatePTX(const std::string &source,
                        const std::string &countParameter) {
  auto program = Parser(Lexer(source).tokenize()).parse();
  auto kernel = IRGenerator().generate(program);

  foldConstants(kernel);
  eliminateDeadCode(kernel);

  return PTXGenerator().generate(kernel, countParameter);
}

void contains(const std::string &text, const std::string &expected) {
  require(text.find(expected) != std::string::npos,
          "Missing PTX text: " + expected);
}

void testArrayKernel() {
  const auto ptx = generatePTX("kernel calculate(int* a, int* output, int n) {"
                               "  output[i] = a[i] + (10 * 4);"
                               "}",
                               "n");

  contains(ptx, ".visible .entry mgpu_calculate(");
  contains(ptx, ".param .u64 p0");
  contains(ptx, ".param .s32 p2");
  contains(ptx, "ld.param.s32 %r2, [p2]");
  contains(ptx, "cvta.to.global.u64 %rd0, %rd0");
  contains(ptx, "mad.lo.u32 %index");
  contains(ptx, "setp.le.s32 %exit, %r2, 0");
  contains(ptx, "setp.ge.u32 %exit, %index, %r2");
  contains(ptx, "mul.wide.s32 %offset");
  contains(ptx, "ld.global.s32");
  contains(ptx, "add.s32");
  contains(ptx, "st.global.s32");
  contains(ptx, ", 40;");
  contains(ptx, "MGPU_DONE:\n  ret;");

  require(ptx.find("mul.lo.s32") == std::string::npos,
          "Constant multiplication should already be folded");

  require(ptx.find("@%exit bra MGPU_DONE;") < ptx.find("ld.global.s32"),
          "Thread guard must precede array accesses");
}

void testDifferentParametersAndArithmetic() {
  const auto ptx = generatePTX(
      "kernel arithmetic(int length, int* input, int factor, int* result) {"
      "  int value = -input[i];"
      "  result[i] = (value * factor - length) / 2;"
      "}",
      "length");

  contains(ptx, "mgpu_arithmetic");
  contains(ptx, "setp.ge.u32 %exit, %index, %r0");
  contains(ptx, "neg.s32");
  contains(ptx, "mul.lo.s32");
  contains(ptx, "sub.s32");
  contains(ptx, "div.s32");
  contains(ptx, "add.u64 %address, %rd1");
  contains(ptx, "add.u64 %address, %rd3");
}

void testRejectedCount(const std::string &name,
                       const std::string &expectedError) {
  bool rejected = false;

  try {
    generatePTX("kernel k(int* data, int n) {}", name);
  } catch (const std::runtime_error &error) {
    contains(error.what(), expectedError);
    rejected = true;
  }

  require(rejected, "Invalid count parameter should be rejected");
}

int main() {
  try {
    testArrayKernel();
    testDifferentParametersAndArithmetic();

    testRejectedCount("missing", "Unknown element count parameter");
    testRejectedCount("data", "Element count parameter must be int");

    std::cout << "All PTX text tests passed\n";
  } catch (const std::exception &error) {
    std::cerr << "PTX test failed: " << error.what() << "\n";
    return 1;
  }

  return 0;
}