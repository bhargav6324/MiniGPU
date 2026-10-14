cmake_minimum_required(VERSION 3.16)

project(MiniGPU VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core compiler library
add_library(minigpu_core
    src/lexer.cpp
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