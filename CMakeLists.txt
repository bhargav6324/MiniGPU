cmake_minimum_required(VERSION 3.16)

project(MiniGPU VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core compiler library
add_library(minigpu_core
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/ir.cpp
    src/ir_generator.cpp
    src/optimizer.cpp
    src/ptx_generator.cpp
)

# Header files
target_include_directories(minigpu_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compiler warnings
target_compile_options(minigpu_core PRIVATE
    -Wall
    -Wextra
    -Werror
)

# MiniGPU executable
add_executable(minigpu
    src/main.cpp
)

# Link compiler components
target_link_libraries(minigpu
    PRIVATE minigpu_core
)

# Enable automated testing
enable_testing()

# Lexer tests
add_executable(test_lexer
    tests/test_lexer.cpp
)

target_link_libraries(test_lexer
    PRIVATE minigpu_core
)

add_test(
    NAME LexerTests
    COMMAND test_lexer
)

add_executable(test_phase2 tests/test_phase2.cpp)
target_link_libraries(test_phase2 PRIVATE minigpu_core)
add_test(NAME Phase2Tests COMMAND test_phase2)

add_executable(test_ir tests/test_ir.cpp)
target_link_libraries(test_ir PRIVATE minigpu_core)
add_test(NAME IRTests COMMAND test_ir)

add_executable(test_optimizer tests/test_optimizer.cpp)
target_link_libraries(test_optimizer PRIVATE minigpu_core)
add_test(NAME OptimizerTests COMMAND test_optimizer)

add_executable(test_dce tests/test_dce.cpp)
target_link_libraries(test_dce PRIVATE minigpu_core)
add_test(NAME DCETests COMMAND test_dce)

add_executable(test_ptx tests/test_ptx.cpp)
target_link_libraries(test_ptx PRIVATE minigpu_core)
add_test(NAME PTXTests COMMAND test_ptx)