# BluePrint Compiler Source

## Why BluePrint

BluePrint was created to enable writing clean, expressive code without repetitive boilerplate or tedious parameter checks. Inspired by the Java Modeling Language (JML), BluePrint aims to make intent and correctness explicit, letting developers focus on core logic rather than defensive programming.

## Planned Structure

This directory will contain the C++ source code for the BluePrint compiler.

- `main.cpp` - Main entry point
- `lexer/` - Lexical analyzer
- `parser/` - Parser implementation  
- `ast/` - Abstract syntax tree definitions
- `codegen/` - LLVM code generation
- `utils/` - Utility functions and error handling

## Implementation Notes

When implementing the compiler, follow these guidelines:
- Use modern C++17 features
- Implement proper error handling
- Add comprehensive unit tests
- Document all public bundles
