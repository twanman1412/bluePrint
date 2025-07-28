# Blueprint Specifications

Blueprints are BluePrint's core feature - they define behavioral contracts for classes and methods, similar to JML but as a first-class language construct.

## Basic Blueprint Syntax

### Blueprint Declaration Structure

```blueprint
blueprint BlueprintName {
    public methodName(param1, param2) {
        input:
            param1: Type1,
            param2: Type2;
        output: ReturnType
        requires: precondition_expression;
        ensures: postcondition_expression;
    }
}
```

### Simple Blueprint Example

```blueprint
blueprint BankAccount {
    public deposit(amount) {
        input: amount: f64;
        output: void;
        requires: amount > 0.0;
        ensures: balance == old(balance) + amount;
    }
    
    public withdraw(amount) {
        input: amount: f64;
        output: bool;
        requires: amount > 0.0 && amount <= balance;
        ensures: (withdraw == true) ==> (balance == old(balance) - amount);
        ensures: (withdraw == false) ==> (balance == old(balance));
    }
    
    public getBalance() {
        output: f64;
        ensures: getBalance == balance && getBalance >= 0.0;
    }
}
```

## Blueprint vs Blueprintless Classes

### Blueprintless Classes
Some classes, particularly system entry points and simple utilities, may not need behavioral contracts:

```blueprint
// Simple class without blueprint specification
class HelloWorld {
   public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}

class Utilities {
    public static String getCurrentTime() {
        return new Date().toString();
    }
}
```

### Classes with Blueprint Contracts
Most classes should implement blueprints for reliability and documentation:

```blueprint
blueprint Calculator {
    public add(a, b) {
        input: a: f64, b: f64;
        output: f64
        ensures: add == a + b;
    }
}

class SimpleCalculator : Calculator {
    public f64 add(f64 a, f64 b) {
        return a + b;
    }
}
```

## Method Overloading in Blueprints

Since blueprints define behavioral contracts separately from implementation signatures, method overloading uses explicit overload syntax.

### Explicit Overload Sets

Define multiple signatures for the same logical operation using explicit overload syntax:

```blueprint
blueprint Printer {
    public print {
        overload (value: i32) {
            output: void
        }
        
        overload (text: String) {
            output: void
            requires: text != null;
        }
        
        overload (value: f64, precision: u32) {
            output: void
            requires: precision <= 10;
            ensures: /* formatted with specified precision */;
        }
    }
}

class ConsolePrinter : Printer {
    public void print(i32 value) {
        System.out.println(value);
    }
    
    public void print(String text) {
        System.out.println(text);
    }
    
    public void print(f64 value, u32 precision) {
        System.out.printf("%.*f", precision, value);
    }
}
```

### Implicit Signature Mapping

The compiler automatically maps implementation methods to blueprint overloads based on exact signature matching - no naming tricks required:

```blueprint
blueprint Calculator {
    public compute {
        overload (a: f64, b: f64) {
            output: f64
            ensures: compute == a + b;
        }
        
        overload (values: f64[]) {
            output: f64
            requires: values.length > 0;
            ensures: /* sum of all values */;
        }
    }
}

class BasicCalculator : Calculator {
    // Automatically maps to first overload
    public f64 compute(f64 a, f64 b) {
        return a + b;
    }
    
    // Automatically maps to second overload  
    public f64 compute(f64[] values) {
        f64 sum = 0.0;
        for (f64 value : values) {
            sum += value;
        }
        return sum;
    }
}
```

## Blueprint Declaration

### Basic Structure

```blueprint
blueprint BlueprintName {
    public methodName(param1, param2) {
        input:
            param1: Type1,
            param2: Type2;
        output: ReturnType
        requires: precondition;
        ensures: postcondition;
    }
}
```

### Blueprint Components

#### Input Specification
Defines the parameters and their types:
```blueprint
input:
    x: f64,
    y: f64,
    name: String;
```

#### Output Specification
Defines the return type:
```blueprint
output: f64;        // Single return value
output: void;       // No return value
```

#### Throws Specification
Defines what exceptions can be thrown:
```blueprint
throws: IOException, IllegalArgumentException;
throws: NetworkException;
throws: /* no exceptions - can be omitted */;
```

#### Requires Clause (Preconditions)
Specifies what must be true when the method is called:
```blueprint
requires: x > 0.0 && y > 0.0;
requires: name != null && name != "";
requires: index >= 0 && index < this.size();
```

#### Ensures Clause (Postconditions)
Specifies what must be true when the method returns normally (no exception):
```blueprint
ensures: result >= 0.0;
ensures: this.size() == old(this.size()) + 1;
ensures: balance == old(balance) - amount;
```

#### Default Values
Specifies default return values for certain input conditions, eliminating the need for redundant base case checking:
```blueprint
default: n <= 0 ==> 0;
default: n == 1 ==> 1;
default: input.isEmpty() ==> emptyResult();
```

#### Exceptional Postconditions
Specifies what must be true when specific exceptions are thrown:
```blueprint
ensures: (throws IOException) ==> (file remains unchanged);
ensures: (throws IllegalArgumentException) ==> (state == old(state));
```

## Advanced Blueprint Features

### Default Values and Base Cases

Default values completely eliminate the need for base case checking in implementations. When a default condition is met, that value is returned automatically:

```blueprint
blueprint MathUtils {
    public fibonacci(n) {
        input: n: u32;
        output: u32
        default: n == 0 ==> 0;      // If n == 0, return 0
        default: n == 1 ==> 1;      // If n == 1, return 1
        default: n == 2 ==> 1;      // If n == 2, return 1
        ensures: (n > 2) ==> (fibonacci == fibonacci(n-1) + fibonacci(n-2));
    }
}

class MathCalculator : MathUtils {
    public u32 fibonacci(u32 n) {
        // No base case checking needed - defaults handle it automatically
        // This code only executes when n > 2
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}
```

#### Multiple Default Priority Rules

When multiple defaults could apply, the **first matching default** is used (top-to-bottom order):

```blueprint
blueprint StringProcessor {
    public processString(input) {
        input: input: str;
        output: str
        default: input == null ==> "";          // Priority 1: null check first
        default: input.length == 0 ==> "empty"; // Priority 2: empty check second
        default: input.length == 1 ==> input.toUpperCase(); // Priority 3: single char
        requires: input != null;
        ensures: processString.length > 0;
    }
}

// Usage examples:
// processString(null) → ""           (matches first default)
// processString("") → "empty"        (matches second default)  
// processString("a") → "A"           (matches third default)
// processString("hello") → [implementation code runs]
```

**Important**: If the desired behavior requires different ordering, the programmer must adjust the default order accordingly.

## Compilation Model

### Blueprint Processing

Blueprints are not compiled as separate units. Instead, they function like C++ header files:

1. **Blueprint Parsing**: Blueprint files are parsed to extract contract information
2. **Contract Injection**: Relevant contract parts are injected into implementing classes during compilation
3. **Verification**: The compiler verifies that class implementations satisfy blueprint contracts
4. **Code Generation**: Final executable code includes both implementation and contract checking code

```blueprint
// fibonacci.blueprint - Not compiled separately
blueprint MathUtils {
    public fibonacci(n) {
        input: n: u32;
        output: u32;
        default: n == 0 ==> 0;
        default: n == 1 ==> 1;
        requires: n >= 0;
        ensures: fibonacci >= 0;
    }
}

// calculator.bp - Compiled with injected contracts
class Calculator : MathUtils {
    public u32 fibonacci(u32 n) {
        // Compiler injects:
        // 1. Precondition checks: assert(n >= 0)
        // 2. Default value handling: if (n == 0) return 0; if (n == 1) return 1;
        // 3. Implementation code (only runs if no defaults match):
        return fibonacci(n - 1) + fibonacci(n - 2);
        // 4. Postcondition checks: assert(result >= 0)
    }
}
```

### Contract Verification Phases

#### 1. Compile-Time Verification
The compiler attempts to verify contracts statically when possible:

```blueprint
class BadImplementation : MathUtils {
    public u32 fibonacci(u32 n) {
        return -1;  // COMPILE ERROR: Cannot guarantee "ensures: fibonacci >= 0"
    }
}

class GoodImplementation : MathUtils {
    public u32 fibonacci(u32 n) {
        return fibonacci(n - 1) + fibonacci(n - 2);  // COMPILE SUCCESS: Can verify contract
    }
}
```

#### 2. Runtime Verification
When compile-time verification isn't possible, contracts are checked at runtime:

```blueprint
class RuntimeChecked : MathUtils {
    public u32 fibonacci(u32 n) {
        u32 result = complexCalculation(n);  // Can't verify statically
        return result;  // Runtime check: assert(result >= 0)
    }
}
```

### Error Reporting

#### Compile-Time Contract Violations
```
Error: Contract violation in Calculator.fibonacci()
  Contract: ensures: fibonacci >= 0
  Violation: Method can return negative value (-1)
  Location: calculator.bp:15:16
  Suggestion: Ensure all return paths satisfy the postcondition
```

#### Runtime Contract Violations  
```
BluePrintException: Precondition violated in Calculator.fibonacci()
  Contract: requires: n >= 0
  Violation: n = -5
  Call Stack:
    at Calculator.fibonacci(calculator.bp:12)
    at Application.main(app.bp:8)
```

### Debug Mode

Enable detailed contract checking information:

```bash
# Compile with contract debugging
blueprintc --debug-contracts calculator.bp

# Output shows all contract checks:
# Checking precondition: n >= 0 ✓
# Checking default: n == 0 ✗  
# Checking default: n == 1 ✗
# Executing implementation code
# Checking postcondition: fibonacci >= 0 ✓
```

### Fractional Type Support in Blueprints

```blueprint
blueprint FractionalMath {
    public divide(numerator, denominator) {
        input:
            numerator: i32,
            denominator: i32;
        output: fractional
        requires: denominator != 0;
        default: numerator == 0 ==> 0/1;
        default: denominator == 1 ==> numerator/1;
        ensures: divide == numerator/denominator;
    }
    
    public simplify(frac) {
        input: frac: fractional;
        output: fractional
        default: frac.denominator == 1 ==> frac;
        ensures: simplify.gcd(numerator, denominator) == 1;
        ensures: simplify == frac;  // Mathematically equivalent
    }
}
```

### Error Handling in Blueprints

```blueprint
blueprint FileManager {
    public readFile(filename) {
        input: filename: String;
        output: String
        throws: IOException, FileNotFoundException;
        requires: filename != null && !filename.isEmpty();
        ensures: readFile != null;
        ensures: (throws FileNotFoundException) ==> (!fileExists(filename));
        ensures: (throws IOException) ==> (fileExists(filename));
    }
    
    public writeFile(filename, content) {
        input:
            filename: String,
            content: String;
        output: void
        throws: IOException, SecurityException;
        requires: filename != null && content != null;
        ensures: fileExists(filename) && fileContent(filename) == content;
        ensures: (throws SecurityException) ==> (!hasWritePermission(filename));
    }
}

blueprint BankAccount {
    public withdraw(amount) {
        input: amount: f64;
        output: void
        throws: InsufficientFundsException, InvalidAmountException;
        requires: amount > 0.0;
        ensures: balance == old(balance) - amount;
        ensures: (throws InsufficientFundsException) ==> (amount > old(balance));
        ensures: (throws InvalidAmountException) ==> (amount <= 0.0);
    }
}
```

### Method Overloading with Different Error Specifications

```blueprint
blueprint DataProcessor {
    public process {
        overload (data: String) {
            output: ProcessedData
            throws: ParseException;
            requires: data != null;
            ensures: process != null;
        }
        
        overload (data: byte[]) {
            output: ProcessedData
            throws: IOException, CorruptDataException;
            requires: data != null && data.length > 0;
            ensures: process != null;
        }
        
        overload (data: InputStream) {
            output: ProcessedData
            throws: IOException, TimeoutException;
            requires: data != null;
            ensures: process != null;
            ensures: data.isClosed();
        }
    }
}
```

### Old Values
Reference previous values in postconditions:
```blueprint
blueprint Stack<T> {
    public push(item) {
        input: item: T;
        output: void;
        ensures: this.size() == old(this.size()) + 1;
        ensures: this.top() == item;
    }
    
    public pop() {
        output: T
        requires: this.size() > 0;
        ensures: this.size() == old(this.size()) - 1;
        ensures: pop == old(top());
    }
}
```

### Conditional Postconditions
```blueprint
blueprint Search {
    public find(array, target) {
        input:
            array: i32[],
            target: i32;
        output: i32
        ensures: (find >= 0) ==> (array[find] == target);
        ensures: (find == -1) ==> (forall i: 0 <= i < array.length ==> array[i] != target);
    }
}
```

### Quantifiers
```blueprint
blueprint Sorting {
    public sort(array) {
        input: array: i32[];
        output: void
        ensures: forall i: 0 <= i < array.length - 1 ==> array[i] <= array[i + 1];
        ensures: array.length == old(array.length);
    }
}
```

### Class Invariants
```blueprint
blueprint BankAccount {
    invariant: balance >= 0.0;
    invariant: accountNumber > 0;
    
    public withdraw(amount) {
        input: amount: f64;
        output: bool
        requires: amount > 0.0;
        ensures: (withdraw == true) ==> (balance >= 0.0);
    }
}
```

## Blueprint Inheritance

Blueprints can extend other blueprints:

```blueprint
blueprint Shape {
    public area() {
        output: f64
        ensures: area >= 0.0;
    }
    
    public perimeter() {
        output: f64
        ensures: perimeter >= 0.0;
    }
}

blueprint Rectangle : Shape {
    public area() {
        output: f64
        ensures: area == width * height;
        ensures: area >= 0.0;  // Inherited constraint
    }
    
    public setWidth(w) {
        input: w: f64;
        output: void
        requires: w > 0.0;
        ensures: width == w;
    }
}
```

## Generic Blueprints

```blueprint
blueprint Container<T> {
    public add(item) {
        input: item: T;
        output: void
        ensures: this.contains(item);
        ensures: this.size() == old(this.size()) + 1;
    }
    
    public remove(item) {
        input: item: T;
        output: bool
        ensures: (remove == true) ==> (!this.contains(item));
        ensures: (remove == true) ==> (this.size() == old(this.size()) - 1);
        ensures: (remove == false) ==> (this.size() == old(this.size()));
    }
    
    public contains(item) {
        input: item: T;
        output: bool
    }
    
    public size() {
        output: u32;
        ensures: size >= 0;
    }
}
```

## Contract Checking

### Compile-time Verification
The BluePrint compiler attempts to verify contracts at compile time when possible. It can provide warnings for potential violations and errors for definite violations:

```blueprint
blueprint BadMath {
    public badMax(a, b) {
        input:
            a: i32,
            b: i32;
        output: i32
        ensures: badMax >= a && badMax >= b;
    }
}

class IncorrectMath : BadMath {
    public i32 badMax(i32 a, i32 b) {
        return a - b;  // COMPILE ERROR: Can't guarantee result >= a && result >= b
    }
}

class MaybeBadMath : BadMath {
    public i32 badMax(i32 a, i32 b) {
        if (someComplexCondition()) {
            return a > b ? a : b;  // Correct implementation
        } else {
            return a - b;  // COMPILE WARNING: Might violate contract
        }
    }
}

blueprint CorrectMath {
    public max(a, b) {
        input:
            a: i32,
            b: i32;
        output: i32
        ensures: max >= a && max >= b;
        ensures: max == a || max == b;
    }
}

class ProperMath : CorrectMath {
    public i32 max(i32 a, i32 b) {
        return a > b ? a : b;  // Compiler can verify this satisfies the contract
    }
}
```

### Runtime Verification
When compile-time verification isn't possible, contracts are checked at runtime with exact values:

```blueprint
blueprint FileReader {
    public readFile(filename) {
        input: filename: str;
        output: str;
        requires: filename != null;
        requires: fileExists(filename);  // Runtime check with actual filename
        ensures: readFile != null;
    }
}

class SafeFileReader : FileReader {
    public str readFile(str filename) {
        // Runtime contract checking:
        // 1. Check filename != null (precondition)
        // 2. Check fileExists(filename) (precondition)
        // 3. Execute method
        // 4. Check readFile != null (postcondition)
        // 5. If any check fails, throw BluePrintException
        
        return FileSystem.readAllText(filename);
    }
}
```

### Contract Violation Handling

When a contract is violated at runtime, a `BluePrintException` is thrown:

```blueprint
class Example {
    public void demonstrateContractViolation() {
        SafeFileReader reader = new SafeFileReader();
        
        try {
            str content = reader.readFile(null); // Violates precondition
        } catch (BluePrintException e) {
            System.err.println("Contract violation: " + e.getMessage());
            // Message might be: "Precondition violated: filename != null"
        } catch (IOException e) {
            System.err.println("IO error: " + e.getMessage());
        }
    }
}
```

### Assertion Modes
- `--contracts=on`: All contracts checked, program terminates on violation (default for debug builds)
- `--contracts=off`: No runtime contract checking (production builds for performance)
