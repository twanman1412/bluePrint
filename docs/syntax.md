# Syntax

BluePrint uses C-style syntax with familiar constructs from languages like Java and C++.

## Comments

```blueprint
// Single-line comment

/*
   Multi-line comment
   can span multiple lines
*/

/**
 * Documentation comment
 * Used for generating documentation
 */
```

## Variables and Declarations

### Variable Declaration

```blueprint
// Basic variable declaration
i32 x = 10;
str name = "Blueprint";
bool isActive = true;

// Type inference (optional)
var count = 42;        // inferred as i32
var message = "Hello"; // inferred as str
```

### Constants

```blueprint
final int MAX_SIZE = 100;
final String APP_NAME = "Blueprint Compiler";
```

## Blueprint Declarations

### Basic Blueprint Syntax

```blueprint
blueprint MyInterface {
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

### Blueprint Examples

```blueprint
blueprint BankAccount {
    public deposit(amount) {
        input: amount: f64;
        output: void
        requires: amount > 0.0;
        ensures: balance == old(balance) + amount;
    }
    
    public withdraw(amount) {
        input: amount: f64;
        output: bool
        requires: amount > 0.0 && amount <= balance;
        ensures: (withdraw == true) ==> (balance == old(balance) - amount);
        ensures: (withdraw == false) ==> (balance == old(balance));
    }
    
    public getBalance() {
        output: f64
        ensures: getBalance == balance && getBalance >= 0.0;
    }
}
```

## Class Declarations

### Basic Class Syntax

```blueprint
class ClassName : BlueprintName1, BlueprintName2 {
    // Field declarations
    private Type fieldName;
    
    // Constructor
    public ClassName(Type param) {
        this.fieldName = param;
    }
    
    // Destructor (called automatically by garbage collector)
    ~ClassName() {
        // Cleanup code - called before garbage collection
        // Destructors of fields are called automatically after this
    }
    
    // Method implementations
    public ReturnType methodName(Type param) {
        // implementation
    }
}
```

### RAII and Destructors

BluePrint follows RAII (Resource Acquisition Is Initialization) principles with automatic destructor chaining:

```blueprint
class FileManager {
    private InputStream inputStream;
    private OutputStream outputStream;
    private str filename;
    
    public FileManager(str filename) {
        this.filename = filename;
        this.inputStream = new FileInputStream(filename);
        this.outputStream = new FileOutputStream(filename + ".out");
    }
    
    // Destructor automatically called during garbage collection
    ~FileManager() {
        System.out.println("Cleaning up FileManager for: " + filename);
        
        // Manual cleanup of resources
        if (inputStream != null) {
            inputStream.close();
        }
        if (outputStream != null) {
            outputStream.close();
        }
        
        // Destructors of inputStream and outputStream called automatically after this
    }
    
    public void processFile() {
        // Use streams...
    }
}
```

### Class with Multiple Inheritance

```blueprint
blueprint Drawable {
    public draw() {
        output: void
    }
}

blueprint Movable {
    public move(x, y) {
        input: 
            x: f64,
            y: f64;
        output: void
        requires: x >= 0.0 && y >= 0.0;
    }
}

class Shape : Drawable, Movable {
    private f64 posX, posY;
    
    public void draw() {
        // Drawing implementation
    }
    
    public void move(f64 x, f64 y) {
        this.posX = x;
        this.posY = y;
    }
}
```

## Basic Types

### Primitive Types
```blueprint
// Integer types
i8 smallInt = 42;      // 8-bit signed integer
i16 mediumInt = 1000;  // 16-bit signed integer  
i32 regularInt = 42;   // 32-bit signed integer
i64 bigInt = 123456;   // 64-bit signed integer

u8 byte = 255;         // 8-bit unsigned integer
u16 word = 65535;      // 16-bit unsigned integer
u32 dword = 42;        // 32-bit unsigned integer  
u64 qword = 123456;    // 64-bit unsigned integer

// Floating point types
f32 price = 19.99f;    // 32-bit floating point
f64 precise = 3.14159; // 64-bit floating point

// Arbitrary precision fraction
fractional half = 1/2;
fractional third = 1/3;
fractional exact = half + third;  // Exactly 5/6

// Other primitives
bool flag = true;      // Boolean type
char letter = 'A';     // Character type
str text = "Hello";    // Basic string (char array)
```

### Reference Types and Collections
```blueprint
// Strings and character arrays
str basicText = "Hello World";          // Character array (str == char[])
String richText = new String("Hello").concat(" World"); // Rich string object

// Fixed-size arrays (size determined at instantiation)
i32[] numbers = {1, 2, 3, 4, 5};        // Array of 5 integers (fixed)
str[] names = new str[10];              // Uninitialized array of 10 strings (fixed)
f64[] coordinates = {1.0, 2.0, 3.0};    // Array of 3 floats (fixed)

// References (for passing by reference, no arithmetic)
i32* numberRef = &someInteger;          // Reference to an integer
str* stringRef = &someString;           // Reference to a string
void* genericRef = null;                // Generic reference

// Reference operations (no arithmetic allowed)
i32 value = *numberRef;                 // Dereference to get value
*numberRef = 42;                        // Set value through reference

// Dynamic collections (use standard library classes)
List<i32> intList = new ArrayList<i32>();
Map<str, i32> nameToAge = new HashMap<str, i32>();
Set<String> uniqueWords = new HashSet<String>();
```

### Generics
```blueprint
blueprint Container<T> {
    public add(item) {
        input: item: T;
        output: void;
    }
    
    public get(index) {
        input: index: u32;
        output: T;
        requires: index < size();
    }
    
    public size() {
        output: u32;
        ensures: size >= 0;
    }
}

class ArrayList<T> : Container<T> {
    private T[] items;
    private u32 count;
    
    public void add(T item) {
        // implementation
    }
    
    public T get(u32 index) {
        return items[index];
    }
    
    public u32 size() {
        return count;
    }
}
```

### Arithmetic Operators
```blueprint
i32 a = 10, b = 3;
i32 sum = a + b;        // Addition: 13
i32 diff = a - b;       // Subtraction: 7
i32 product = a * b;    // Multiplication: 30
i32 quotient = a / b;   // Division: 3
i32 remainder = a % b;  // Modulo: 1
```

### Comparison Operators
```blueprint
bool equal = (a == b);     // Equality
bool notEqual = (a != b);  // Inequality
bool less = (a < b);       // Less than
bool greater = (a > b);    // Greater than
bool lessEq = (a <= b);    // Less than or equal
bool greaterEq = (a >= b); // Greater than or equal
```

### Logical Operators
```blueprint
bool result = (a > 0) && (b > 0);  // Logical AND
bool result2 = (a < 0) || (b < 0); // Logical OR
bool result3 = !(a == b);          // Logical NOT
```

## Control Flow

### If-Else Statements
```blueprint
if (condition) {
    // code block
} else if (anotherCondition) {
    // code block
} else {
    // code block
}
```

### Switch Statements
```blueprint
switch (value) {
    case 1:
        // code
        break;
    case 2:
        // code
        break;
    default:
        // default case
        break;
}
```

### Loops

#### While Loop
```blueprint
while (condition) {
    // loop body
}
```

#### For Loop
```blueprint
for (i32 i = 0; i < 10; i++) {
    // loop body
}

// Enhanced for loop
for (i32 item : array) {
    // iterate over array
}

// Example with proper array types
i32[] values = {1, 2, 3, 4, 5};
for (i32 value : values) {
    System.out.println(value);
}
```

## Keywords

Reserved keywords in BluePrint:

| Keyword | Purpose | Keyword | Purpose |
|---------|---------|---------|---------|
| `blueprint` | Blueprint declaration | `class` | Class declaration |
| `interface` | Interface declaration | `enum` | Enumeration |
| `public` | Public access | `private` | Private access |
| `protected` | Protected access | `static` | Static member |
| `final` | Final/constant | `abstract` | Abstract class/method |
| `input` | Blueprint input spec | `output` | Blueprint output spec |
| `requires` | Precondition | `ensures` | Postcondition |
| `default` | Default return value | `throws` | Exception specification |
| `invariant` | Class invariant | `old` | Previous value reference |
| `if` | Conditional | `else` | Alternative condition |
| `while` | While loop | `for` | For loop |
| `do` | Do-while loop | `switch` | Switch statement |
| `case` | Switch case | `default` | Default case |
| `break` | Break statement | `continue` | Continue statement |
| `return` | Return value | `void` | No return value |
| `match` | Pattern matching | `try` | Exception handling |
| `catch` | Exception catch | `finally` | Finally block |
| `throw` | Throw exception | `throws` | Method exception spec |
| `i8`, `i16`, `i32`, `i64` | Signed integers | `u8`, `u16`, `u32`, `u64` | Unsigned integers |
| `f32`, `f64` | Floating point | `fractional` | Arbitrary precision fraction |
| `bool` | Boolean type | `char` | Character type |
| `str` | Basic string type | `String` | Rich string type |
| `true` | Boolean true | `false` | Boolean false |
| `null` | Null reference | `new` | Object creation |
| `this` | Current object | `super` | Parent class |
| `extends` | Single inheritance | `implements` | Interface implementation |
| `import` | Import statement | `package` | Package declaration |
| `async` | Async method | `await` | Await async result |
| `synchronized` | Synchronized block | `volatile` | Volatile field |
