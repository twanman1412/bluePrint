# Blueprint Specifications

Blueprints are BluePrint's core feature - they define behavioral contracts for classes and methods, similar to JML but as a first-class language construct.

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

## Built-in System Blueprints

The BluePrint standard library provides comprehensive blueprints for common patterns and system interactions:

### Core System Blueprints

```blueprint
// Application entry point
blueprint System.Application {
    public static main(args) {
        input: args: str[];
        output: void
        requires: args != null;
    }
}

// Comparable interface for ordering
blueprint System.Comparable<T> {
    public compareTo(other) {
        input: other: T;
        output: i32
        requires: other != null;
        ensures: (compareTo == 0) <==> this.equals(other);
        ensures: (compareTo > 0) <==> other.compareTo(this) < 0;
        ensures: (compareTo < 0) <==> other.compareTo(this) > 0;
    }
}

// Object equality and hashing
blueprint System.Object {
    public equals(other) {
        input: other: Object;
        output: bool
        ensures: equals(this) == true;  // Reflexive
        ensures: equals(other) == other.equals(this);  // Symmetric
    }
    
    public hashCode() {
        output: i32
        ensures: this.equals(other) ==> (hashCode() == other.hashCode());
    }
    
    public toString() {
        output: String
        ensures: toString != null;
    }
}
```

### Collection Blueprints

```blueprint
// Basic iterable collection
blueprint System.Collection<T> {
    public size() {
        output: u32
        ensures: size >= 0;
    }
    
    public isEmpty() {
        output: bool
        ensures: isEmpty <==> (size() == 0);
    }
    
    public contains(item) {
        input: item: T;
        output: bool
    }
    
    public iterator() {
        output: Iterator<T>
        ensures: iterator != null;
    }
}

// List with indexed access
blueprint System.List<T> : System.Collection<T> {
    public get(index) {
        input: index: u32;
        output: T
        throws: IndexOutOfBoundsException;
        requires: index < size();
        ensures: (throws IndexOutOfBoundsException) ==> (index >= size());
    }
    
    public set(index, item) {
        input:
            index: u32,
            item: T;
        output: void
        throws: IndexOutOfBoundsException;
        requires: index < size();
        ensures: get(index) == item;
    }
    
    public add(item) {
        input: item: T;
        output: void
        ensures: size() == old(size()) + 1;
        ensures: contains(item);
    }
    
    public remove(index) {
        input: index: u32;
        output: T
        throws: IndexOutOfBoundsException;
        requires: index < size();
        ensures: size() == old(size()) - 1;
    }
}

// Map/dictionary interface
blueprint System.Map<K, V> {
    public put(key, value) {
        input:
            key: K,
            value: V;
        output: V?  // Previous value or null
        requires: key != null;
        ensures: get(key) == value;
        ensures: containsKey(key);
    }
    
    public get(key) {
        input: key: K;
        output: V?
        requires: key != null;
        ensures: (get != null) <==> containsKey(key);
    }
    
    public containsKey(key) {
        input: key: K;
        output: bool
        requires: key != null;
    }
    
    public size() {
        output: u32
        ensures: size >= 0;
    }
}
```

### I/O and Resource Management

```blueprint
// File operations
blueprint System.File {
    public exists() {
        output: bool
    }
    
    public isFile() {
        output: bool
        ensures: isFile() ==> exists();
    }
    
    public isDirectory() {
        output: bool
        ensures: isDirectory() ==> exists();
        ensures: !(isFile() && isDirectory());
    }
    
    public canRead() {
        output: bool
        ensures: canRead() ==> exists();
    }
    
    public canWrite() {
        output: bool
        ensures: canWrite() ==> exists();
    }
}

// Input stream operations
blueprint System.InputStream {
    public read() {
        output: i32  // -1 for end of stream
        throws: IOException;
        ensures: (read >= 0 && read <= 255) || read == -1;
    }
    
    public read(buffer) {
        input: buffer: byte[];
        output: i32  // bytes read, -1 for end
        throws: IOException;
        requires: buffer != null;
        ensures: read >= -1 && read <= buffer.length;
    }
    
    public close() {
        output: void
        throws: IOException;
        ensures: isClosed();
    }
    
    public isClosed() {
        output: bool
    }
}

// Auto-closeable resources
blueprint System.AutoCloseable {
    public close() {
        output: void
        throws: Exception;
        ensures: isClosed();
    }
    
    public isClosed() {
        output: bool
    }
}
```

### Threading and Concurrency

```blueprint
// Thread-safe operations
blueprint System.Concurrent.Lock {
    public lock() {
        output: void
        ensures: isHeldByCurrentThread();
    }
    
    public unlock() {
        output: void
        requires: isHeldByCurrentThread();
        ensures: !isHeldByCurrentThread();
    }
    
    public tryLock() {
        output: bool
        ensures: tryLock ==> isHeldByCurrentThread();
    }
    
    public isHeldByCurrentThread() {
        output: bool
    }
}

// Future pattern for async operations
blueprint System.Concurrent.Future<T> {
    public get() {
        output: T
        throws: InterruptedException, ExecutionException;
        requires: isDone();
        ensures: get != null || T is nullable;
    }
    
    public isDone() {
        output: bool
    }
    
    public isCancelled() {
        output: bool
        ensures: isCancelled() ==> isDone();
    }
    
    public cancel(mayInterruptIfRunning) {
        input: mayInterruptIfRunning: bool;
        output: bool
        ensures: cancel ==> isCancelled();
    }
}
```

Usage with built-in blueprints:

```blueprint
class HelloWorld : System.Application {
    public static void main(str[] args) {
        System.out.println("Hello, World!");
    }
}

class Person : System.Comparable<Person> {
    private String name;
    private i32 age;
    
    public i32 compareTo(Person other) {
        i32 nameComparison = this.name.compareTo(other.name);
        if (nameComparison != 0) {
            return nameComparison;
        }
        return this.age - other.age;
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
output: f64        // Single return value
output: void       // No return value
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
requires: name != null && name.length() > 0;
requires: index >= 0 && index < size();
```

#### Ensures Clause (Postconditions)
Specifies what must be true when the method returns normally (no exception):
```blueprint
ensures: result >= 0.0;
ensures: size() == old(size()) + 1;
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

Default values eliminate redundant base case checking in implementations:

```blueprint
blueprint MathUtils {
    public fibonacci(n) {
        input: n: u32;
        output: u32
        default: n <= 0 ==> 0;
        default: n == 1 ==> 1;
        default: n == 2 ==> 1;
        ensures: (n > 2) ==> (fibonacci == fibonacci(n-1) + fibonacci(n-2));
    }
    
    public factorial(n) {
        input: n: u32;
        output: u64
        default: n == 0 ==> 1;
        default: n == 1 ==> 1;
        requires: n <= 20;  // Prevent overflow
        ensures: (n > 1) ==> (factorial == n * factorial(n-1));
    }
    
    public power(base, exponent) {
        input:
            base: f64,
            exponent: i32;
        output: f64
        default: exponent == 0 ==> 1.0;
        default: exponent == 1 ==> base;
        default: base == 0.0 && exponent > 0 ==> 0.0;
        ensures: (exponent > 1) ==> (power == base * power(base, exponent - 1));
    }
}

class MathCalculator : MathUtils {
    public u32 fibonacci(u32 n) {
        // Base cases handled automatically by defaults
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
    
    public u64 factorial(u32 n) {
        // Base cases handled automatically by defaults  
        return n * factorial(n - 1);
    }
    
    public f64 power(f64 base, i32 exponent) {
        // Base cases handled automatically by defaults
        if (exponent < 0) {
            return 1.0 / power(base, -exponent);
        }
        return base * power(base, exponent - 1);
    }
}
```

### Default Values with Complex Conditions

```blueprint
blueprint StringProcessor {
    public processString(input) {
        input: input: str;
        output: str
        default: input == null ==> "";
        default: input.length == 0 ==> "empty";
        default: input.length == 1 ==> input.toUpperCase();
        requires: input != null;
        ensures: processString.length > 0;
    }
    
    public findIndex(array, target) {
        input:
            array: i32[],
            target: i32;
        output: i32
        default: array == null ==> -1;
        default: array.length == 0 ==> -1;
        default: array[0] == target ==> 0;
        ensures: (findIndex >= 0) ==> (array[findIndex] == target);
        ensures: (findIndex == -1) ==> (forall i: 0 <= i < array.length ==> array[i] != target);
    }
}
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
        output: void
        ensures: size() == old(size()) + 1;
        ensures: top() == item;
    }
    
    public pop() {
        output: T
        requires: size() > 0;
        ensures: size() == old(size()) - 1;
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
        ensures: contains(item);
        ensures: size() == old(size()) + 1;
    }
    
    public remove(item) {
        input: item: T;
        output: bool
        ensures: (remove == true) ==> (!contains(item));
        ensures: (remove == true) ==> (size() == old(size()) - 1);
        ensures: (remove == false) ==> (size() == old(size()));
    }
    
    public contains(item) {
        input: item: T;
        output: bool
    }
    
    public size() {
        output: u32
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
        output: str
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
