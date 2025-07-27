# BluePrint Programming Language

Welcome to the BluePrint programming language documentation. BluePrint is a strongly-typed, object-oriented programming language with built-in behavioral specifications called **blueprints**.

## Overview

BluePrint combines the familiar syntax of C-style languages with powerful built-in specification capabilities. The language's defining feature is **blueprints** - behavioral specifications that describe what functions expect, what they guarantee, and how they behave.

### Key Features

- **Built-in Blueprints**: First-class behavioral specifications (inspired by JML)
- **Strong Type System**: Compile-time type safety with inference
- **Object-Oriented**: Classes, inheritance, and polymorphism
- **Concurrency Support**: Built-in threading and synchronization primitives
- **C-style Syntax**: Familiar syntax for developers coming from C, Java, or similar languages

### Design Goals

- **Reliability**: Specifications catch errors at compile-time and runtime
- **Clarity**: Code is self-documenting through blueprints
- **Performance**: Compile to efficient native code via LLVM
- **Safety**: Strong typing and contracts prevent common bugs

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

class Calculator : Calculator {
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
