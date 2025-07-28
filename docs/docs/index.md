# BluePrint Programming Language

Welcome to the BluePrint programming language documentation. BluePrint is a strongly-typed, object-oriented programming language with built-in behavioral specifications called **blueprints**.

## Overview

BluePrint combines the familiar syntax of C-style languages with powerful built-in specification capabilities. The language's defining feature is **blueprints** - behavioral specifications that describe what functions expect, what they guarantee, and how they behave.

### Key Features

- **Built-in Blueprints**: First-class behavioral specifications
- **Strong Type System**: Compile-time type safety with inference
- **Object-Oriented**: Classes, inheritance, and polymorphism
- **Concurrency Support**: Built-in threading and synchronization primitives
- **C-style Syntax**: Familiar syntax for developers coming from C, Java, or similar languages

### Design Goals

- **Reliability**: Specifications catch errors at compile-time and runtime
- **Clarity**: Code is self-documenting through blueprints
- **Performance**: Compile to efficient native code via LLVM
- **Safety**: Strong typing and contracts prevent common bugs
- **Memory Safety**: Automatic garbage collection with RAII-style destructors (see [Memory Management](memory.md))

## Getting Started

### Installation

[Installation instructions will go here]

### Hello, World!

```blueprint
class HelloWorld {
    public static void main(str[] args) {
        System.out.println("Hello, World!");
    }
}
```

### Your First Blueprint

```blueprint
blueprint Calculator {
    public max(a, b) {
        input:
            a: i32,
            b: i32;
        output: i32
        requires: a >= 0 && b >= 0;
        ensures: max >= a && max >= b;
    }
    
    public sqrt(x) {
        input: x: f64;
        output: f64
        requires: x >= 0.0;
        ensures: sqrt * sqrt >= x - 0.001 && sqrt * sqrt <= x + 0.001;
    }
}

class BasicCalculator : Calculator {
    public i32 max(i32 a, i32 b) {
        return a > b ? a : b;
    }
    
    public f64 sqrt(f64 x) {
        // Implementation here
        return Math.sqrt(x);
    }
}
```

### Basic Syntax Overview

BluePrint separates interface specifications (blueprints) from implementations (classes). Blueprints define behavioral contracts using `requires` (preconditions) and `ensures` (postconditions), while classes provide the actual implementation.

## Documentation Structure

This documentation is organized in the following order:

1. **[Introduction](index.md)** - Overview, goals, and getting started
2. **[Syntax](syntax.md)** - Basic language syntax, keywords, and operators
3. **[Type System](types.md)** - Primitive types, generics, and inheritance
4. **[Control Flow](control-flow.md)** - Conditionals, loops, and pattern matching
5. **[Functions](functions.md)** - Method declarations, overloading, and async functions
6. **[Blueprint Specifications](blueprints.md)** - Core feature: contracts and verification
7. **[Memory Management](memory.md)** - Garbage collection, destructors, and RAII
8. **[Concurrency](concurrency.md)** - Async/await, threading, and synchronization
9. **[Exception Handling](exceptions.md)** - Error handling and contract violations
10. **[Modules](modules.md)** - Import system, bundles, and code organization
11. **[Standard Library](stdlib.md)** - Built-in functionality and system blueprints
12. **[Examples](examples.md)** - Practical applications and code samples

Each section builds upon previous concepts, so it's recommended to read them in order when learning the language.
