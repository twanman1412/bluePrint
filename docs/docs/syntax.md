# Syntax

BluePrint uses C-style syntax with familiar constructs from languages like Java and C++. This section covers the basic language syntax elements - for more detailed information on specific features, see their dedicated sections.

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
final i32 MAX_SIZE = 100;
final str APP_NAME = "Blueprint Compiler";
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
    
    // Method implementations
    public ReturnType methodName(Type param) {
        // implementation
        return value;
    }
}
```

### Class with Multiple Inheritance

```blueprint
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

## Basic Types and Literals

### Primitive Type Literals
```blueprint
// Integer literals
i32 regularInt = 42;
i64 bigInt = 123456L;
u32 unsignedInt = 42u;

// Floating point literals  
f32 price = 19.99f;
f64 precise = 3.14159;

// Other literals
bool flag = true;
char letter = 'A';
str text = "Hello World";
```

### Array and Collection Literals
```blueprint
// Fixed-size arrays
i32[] numbers = {1, 2, 3, 4, 5};
str[] names = {"Alice", "Bob", "Charlie"};

// Dynamic collections (requires import)
List<i32> intList = new ArrayList<i32>();
Map<str, i32> nameToAge = new HashMap<str, i32>();
```

## Operators

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

## Basic Control Flow

### Conditional Statements
```blueprint
if (condition) {
    // code block
} else if (anotherCondition) {
    // code block  
} else {
    // code block
}
```

### Loops
```blueprint
// For loop
for (i32 i = 0; i < 10; i++) {
    // loop body
}

// Enhanced for loop
for (i32 item : array) {
    // iterate over array
}

// While loop
while (condition) {
    // loop body
}
```

### Switch Statement
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

## Reserved Keywords

Core language keywords include:

**Declaration keywords**: `blueprint`, `class`, `enum`, `interface`  
**Access modifiers**: `public`, `private`, `protected`, `static`, `final`  
**Blueprint contracts**: `input`, `output`, `requires`, `ensures`, `default`, `throws`, `invariant`  
**Control flow**: `if`, `else`, `while`, `for`, `do`, `switch`, `case`, `break`, `continue`, `return`  
**Exception handling**: `try`, `catch`, `finally`, `throw`  
**Type keywords**: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`, `char`, `str`, `void`  
**Literals**: `true`, `false`, `null`  
**Object operations**: `new`, `this`, `super`  
**Concurrency**: `async`, `await`, `synchronized`, `volatile`

For detailed information on types, see [Type System](types.md). For blueprint contracts, see [Blueprint Specifications](blueprints.md).
