# MiniGPU

MiniGPU is a C++17 compiler for a restricted integer kernel language. It translates source programs into NVIDIA PTX.

## Compilation pipeline

Source → Lexer → Parser → AST → Semantic analysis → IR → Optimization → PTX

The compiler implements its own front end, intermediate representation, optimization passes, and PTX text generator. NVIDIA tools assemble the generated PTX. GPU validation uses CuPy to load and launch it.

## Supported language

```cpp
kernel calculate(int* input, int* output, int count) {
    int value = input[i] + (10 * 4);
    output[i] = value;
}
```

Supported features:

- One kernel per source file.
- Explicit `int` and `int*` parameters.
- Local integer variables and reassignment.
- Multiple statements.
- Integer arithmetic: `+`, `-`, `*`, `/`, unary signs, and parentheses.
- Array reads and writes with integer index expressions.
- A predefined, read-only thread index named `i`.

Semantic analysis checks unknown and duplicate names, uninitialized local variables, scalar and array misuse, assignment to `i`, and integer literal limits.

## Optimizations

**Constant folding** replaces arithmetic on known constants with its result. Calculations that divide by zero or produce values outside the signed 32-bit range are left unchanged.

**Dead-code elimination** removes unused calculations. The current pass conservatively preserves stores, loads, and unresolved divisions.

## Build and test

Requirements: a C++17 compiler and CMake 3.16 or newer.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The project has six test suites:

- LexerTests
- Phase2Tests
- IRTests
- OptimizerTests
- DCETests
- PTXTests

PTXTests check generated text. NVIDIA assembly and GPU execution are validated separately.

## Commands

```sh
./build/minigpu --tokens examples/ptx_demo.mgpu
./build/minigpu --check examples/ptx_demo.mgpu
./build/minigpu --ir examples/ptx_demo.mgpu
./build/minigpu --ir-opt examples/ptx_demo.mgpu

./build/minigpu --ptx examples/ptx_demo.mgpu n
./build/minigpu --ptx-raw examples/ptx_demo.mgpu n
```

`--ptx` applies both optimization passes. `--ptx-raw` skips them.

The final argument selects the integer parameter containing the element count. Its name and position are not fixed. Generated kernels exit when that count is nonpositive or the global thread index is outside its range.

Generated entry names use the prefix `mgpu_`. The current backend emits PTX 7.0 with an `sm_75` target and 64-bit addresses.

## GPU validation

Validation performed in Google Colab includes:

| Program | Checks |
|---|---:|
| Array addition and thread-range guarding | 8 GPU runs |
| Strided indexing, reassignment, and signed arithmetic | 18 GPU runs |
| Unoptimized and optimized execution against CPU results | 16 GPU runs |

These runs passed for the tested inputs. Output padding containing sentinel values checked that excess threads left the surrounding output region unchanged.

PTX assembly was checked using NVIDIA's `ptxas`.

## Current limitations

- No control flow, function calls, floating-point types, local arrays, or multiple kernels per file.
- The thread-range guard does not validate every array index. Expressions such as `input[i * 2]` require a suitably sized allocation.
- No data-race detection or synchronization features.
- No runtime checks for division by zero or arithmetic overflow. GPU validation inputs avoid these cases.
- Validation covers selected programs and inputs; it does not establish correctness for every accepted program.
- No GPU performance improvement has been measured. NVIDIA's downstream compiler also optimizes PTX.