# BluePrint Language Roadmap

This roadmap outlines the iterative development of the BluePrint programming language compiler and interpreter.

## Phase 1: Foundation & Type System (v0.x)
*Goal: Establish the runtime behavior, type system, and basic execution flow.*

### v0.0.0: The Bare Essentials
- [ ] **Primitive Types:** Support for core types `i32`, `f64`, `bool`, `char`.
- [ ] **Variables:** Declaration and assignment logic.
- [ ] **Arithmetic:** Basic operators (`+`, `-`, `*`, `/`, `%`) and precedence logic.
- [ ] **Logic:** Boolean operators (`&&`, `||`, `!`) and comparison (`==`, `!=`, `<`, `>`).
- [ ] **Control Flow:** `if`, `else`, and `while` loop structures.
- [ ] **Entry Point:** Logic to identify and execute `System.Application.main`.

### v0.1.0: Extended Type System
- [ ] **Full Primitive Suite:** Implementation of `i8`-`i64`, `u8`-`u64`, and `f32`.
- [ ] **Fractional Type:** Native support for the `fractional` type (numerator/denominator logic) and its arithmetic promotion rules.
- [ ] **Strings:** Implementation of the `str` type (character array) and string literals.
- [ ] **Arrays:** Fixed-size array syntax (`T[]`), initialization literals, and index access.
- [ ] **Control Flow II:** Implement `do-while`, `for` loops (standard and range-based), and `switch` statements.

### v0.2.0: Object-Oriented Core
- [ ] **Classes:** Syntax for `class`, fields, and methods.
- [ ] **Constructors:** Object initialization logic and memory allocation.
- [ ] **Visibility:** Access modifiers (`public`, `private`, `protected`).
- [ ] **References:** `this` keyword and reference handling.
- [ ] **Inheritance:** Single inheritance logic for classes.

### v0.3.0: Memory & Safety
- [ ] **Memory Management:** Reference counting or GC integration points.
- [ ] **Destructors:** Implementation of destructor syntax and automatic invocation rules.
- [ ] **Null Safety:** Nullable types (`Type?`) and safe navigation operators (`?.`).
- [ ] **Casting:** `instanceof` checks and safe casting (`as`) logic.

### v0.4.0: Generics & Inference
- [ ] **Generics:** Type parameter syntax (`<T>`) for classes.
- [ ] **Generic Methods:** Handling type parameters at the method level.
- [ ] **Bounded Generics:** Constraint logic (e.g., `T : Serializable`).
- [ ] **Inference:** Implementation of `var` for local variable type deduction.

---

## Phase 2: The BluePrint Layer (v1.x)
*Goal: Implement the unique specification and contract features of the language.*

### v1.0.0: Blueprint Architecture
- [ ] **Blueprint Definition:** Syntax for defining `blueprint` (interfaces).
- [ ] **Implementation:** Logic for classes to implement blueprints.
- [ ] **Multiple Inheritance:** Support for implementing multiple blueprints.
- [ ] **Diamond Resolution:** Virtual inheritance logic for shared blueprint ancestry.

### v1.1.0: Contracts - Preconditions
- [ ] **Requires Clause:** Parser and runtime injector for `requires` checks.
- [ ] **Input Spec:** Syntax for `input:` parameter definitions within blueprints.
- [ ] **Default Values:** Logic for `default` guards (e.g., `default: n == 0 ==> 0`) to bypass implementation.

### v1.2.0: Contracts - Postconditions
- [ ] **Ensures Clause:** Parser and runtime injector for `ensures` checks.
- [ ] **Output Spec:** Syntax for `output:` return definitions.
- [ ] **Return Values:** Logic to access the return value within the contract (using function name).
- [ ] **Old Values:** Implementation of `old()` to capture state before method execution.

### v1.3.0: Advanced Specifications
- [ ] **Invariants:** Class-level `invariant` checks injected before/after public method calls.
- [ ] **Quantifiers:** Support for `forall` logic in contracts (loop generation for array checks).
- [ ] **Overloading:** Explicit `overload` set definition in blueprints and mapping to implementation methods.

---

## Phase 3: Functional & Async Features (v2.x)
*Goal: Modernize the language with functional paradigms and concurrency.*

### v2.0.0: Functional Constructs
- [ ] **Lambdas:** Syntax for anonymous functions `(x) => x * x`.
- [ ] **Function Types:** First-class function type signatures (e.g., `Function<T, U>`).
- [ ] **Closures:** Variable capture logic for lambdas.

### v2.1.0: Error Handling
- [ ] **Exception System:** `try`, `catch`, `finally` blocks.
- [ ] **Throws Specification:** `throws` clause in blueprints.
- [ ] **Exceptional Postconditions:** Logic for `ensures (throws Ex) ==> condition`.
- [ ] **Pattern Matching:** `match` syntax for structural pattern matching.

### v2.2.0: Concurrency
- [ ] **Async/Await:** State machine generation for `async` methods.
- [ ] **Future Type:** Native `Future<T>` handling in the compiler.
- [ ] **Synchronization:** `synchronized` keyword support for thread safety.

---

## Phase 4: Ecosystem & Modules (v3.x)
*Goal: Structure the code for large-scale development.*

### v3.0.0: Module System
- [ ] **Imports:** Logic for resolving and linking external files.
- [ ] **File Separation:** Handling of definition files (`.bpf`) vs implementation files (`.bp`).
- [ ] **Bundles:** Package/Bundle manifest parsing and export logic.

### v3.1.0: Compilation Polish
- [ ] **Debug Mode:** Compiler flags to toggle contract verification levels (Runtime vs. Debug).
- [ ] **Static Analysis:** Compile-time verification of simple contracts (e.g., detecting `return -1` when `ensures > 0`).
