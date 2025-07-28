# BluePrint Programming Language

BluePrint is a modern programming language designed for [your specific use case/goals].

## Why BluePrint

BluePrint was created to enable writing clean, expressive code without repetitive boilerplate or tedious parameter checks. Inspired by the Java Modeling Language (JML), BluePrint aims to make intent and correctness explicit, letting developers focus on core logic rather than defensive programming.

## Project Structure

```
bluePrint/
├── src/                    # Compiler source code (C++)
├── tests/                  # Test files
├── docs/                   # Documentation (Markdown files)
├── examples/               # Example BluePrint programs
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## Documentation

The language documentation is written in Markdown and can be generated into a single HTML file.

### Building Documentation

```bash
cd docs
npm install          # Install dependencies
npm run build       # Generate HTML documentation
npm run serve       # Serve documentation locally at http://localhost:3000
```

The generated documentation will be available in `docs/build/index.html`.

### Documentation Files

- `docs/index.md` - Introduction and getting started
- `docs/syntax.md` - Basic language syntax and keywords
- `docs/types.md` - Type system, generics, and inheritance
- `docs/control-flow.md` - Control flow constructs and pattern matching
- `docs/functions.md` - Functions, methods, and async programming
- `docs/blueprints.md` - Blueprint specifications and contracts
- `docs/memory.md` - Memory management, garbage collection, and RAII
- `docs/concurrency.md` - Async/await, threading, and synchronization
- `docs/exceptions.md` - Exception handling and error management
- `docs/modules.md` - Module system, imports, and .bpf files
- `docs/stdlib.md` - Standard library (to be completed)
- `docs/examples.md` - Practical code examples and applications

## Compiler Development

The compiler is implemented in C++ and targets LLVM IR.

### Prerequisites (for when you implement the compiler)

- CMake 3.15 or higher
- LLVM 12.0 or higher
- C++17 compatible compiler (GCC 8+, Clang 8+, MSVC 2019+)

### Build Instructions (for future implementation)

```bash
mkdir build && cd build
cmake ..
make
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.