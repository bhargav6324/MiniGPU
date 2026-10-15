# MiniGPU

MiniGPU is a small C++17 compiler project. The current implementation covers
lexing, parsing into an AST, and semantic checks. It does not yet generate IR
or PTX.

## Phase 2 language

```text
kernel name(int* input, int* output, int n) {
    int offset = i + 1;
    output[offset] = input[offset] * 2 + n;
}
```

Parameters have explicit `int` or `int*` types. Local variables are
`int`, with optional initializers. Statements are declarations or assignments
to scalar names or indexed arrays. Expressions support integer literals,
names, array reads with expression indices, parentheses, unary `+` and `-`,
and binary `+`, `-`, `*`, and `/`. The predefined `i` is a read-only
integer thread index. There are no blocks nested inside the kernel, control
flow statements, calls, or pointer arithmetic yet.

The semantic pass checks duplicate and unknown names, uses before local
initialization, scalar versus array use, assignment to `i`, and integer
literals outside the signed 32-bit range (including the negative minimum).
It does not evaluate
expressions or check runtime bounds and division by zero.

## Build and check

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/minigpu --tokens examples/vector_add.mgpu
./build/minigpu --check examples/vector_add.mgpu
```
