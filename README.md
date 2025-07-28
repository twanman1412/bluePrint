# BluePrint Programming Language

BluePrint is a modern, object oriented programming language designed to be simple, efficient, and expressive. It features a clean syntax, strong static typing, and a powerful standard library. The language's key feature is the blueprint system, which allows developers to define all preconditions, postconditions, and invariants for classes and methods, ensuring robust and reliable code wihtout the fluff of each method having a separate base case, error catching, etc.

## Why BluePrint

BluePrint was created to enable writing clean, expressive code without repetitive boilerplate or tedious parameter checks. Inspired by the Java Modeling Language (JML), BluePrint aims to make intent and correctness explicit, letting developers focus on core logic rather than defensive programming.

## Project Structure

```
bluePrint/
├── docs/                  # Documentation
└── README.md              # This file
```

## Documentation

The language documentation is written in Markdown and can be generated into a single HTML file.

1. Navigate to the `docs` directory.
2. Install the required dependencies using `npm install`.
3. Run the build script with `npm run build` to generate the HTML documentation.
4. The generated documentation will be available in `docs/build/index.html`.

or alternatively, you can serve the documentation locally using `npm run serve`, which will host it at `http://localhost:3000`.

### Documentation Files

- `docs/blueprints.md` - Blueprint system overview
- `docs/concurrency.md` - Concurrency model
- `docs/control-flow.md` - Control flow constructs
- `docs/examples.md` - Code examples
- `docs/exceptions.md` - Exception handling and error management
- `docs/functions.md` - Functions
- `docs/index.md` - Introduction and getting started
- `docs/memory.md` - Memory management, garbage collection, and RAII
- `docs/modules.md` - Module system
- `docs/stdlib.md` - Standard library
- `docs/syntax.md` - Language syntax
- `docs/types.md` - Type system

## Compiler Development

The compiler is implemented in C++ and targets LLVM IR.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
