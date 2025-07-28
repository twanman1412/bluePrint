# Type System

BluePrint features a strong static type system with generics, multiple inheritance, and comprehensive type checking.

## Primitive Types

### Integer Types

| Type | Size | Range | Description |
|------|------|-------|-------------|
| `i8` | 8-bit | -128 to 127 | Signed byte |
| `i16` | 16-bit | -32,768 to 32,767 | Signed short |
| `i32` | 32-bit | -2³¹ to 2³¹-1 | Signed integer |
| `i64` | 64-bit | -2⁶³ to 2⁶³-1 | Signed long |
| `u8` | 8-bit | 0 to 255 | Unsigned byte |
| `u16` | 16-bit | 0 to 65,535 | Unsigned short |
| `u32` | 32-bit | 0 to 2³²-1 | Unsigned integer |
| `u64` | 64-bit | 0 to 2⁶⁴-1 | Unsigned long |

### Floating Point Types

| Type | Size | Description |
|------|------|-------------|
| `f32` | 32-bit | Single precision floating point |
| `f64` | 64-bit | Double precision floating point |

### Other Primitives

| Type | Description |
|------|-------------|
| `bool` | Boolean (true/false) |
| `char` | Unicode character |
| `str` | Basic string type (equivalent to char[]) |
| `void` | No value (for methods that don't return) |

### Arbitrary Precision Types

| Type | Description |
|------|-------------|
| `fractional` | Arbitrary precision fraction (two i32: numerator/denominator) |

```blueprint
// Fractional arithmetic maintains exact precision
fractional half = 1/2;
fractional third = 1/3;
fractional result = half + third;  // Exactly 5/6, not 0.833333...

// Fractional operations with integers (exact arithmetic)
fractional withInt = result + 1;   // Exactly 11/6
fractional doubled = result * 2;   // Exactly 5/3
i32 wholeNumber = 5;
fractional mixed = half + wholeNumber; // Exactly 11/2

// Operations remain exact as long as no floating point is involved
fractional complex = (1/3) * 6 + (2/5); // Exactly 12/5

// Type promotion rules:
// fractional + integer = fractional (exact arithmetic)
// fractional + fractional = fractional (exact arithmetic)  
// fractional + float = float (converts to floating point)
f64 floatValue = 3.14;
f64 converted = result + floatValue;  // Becomes floating point: ~4.9733333...

// Examples of type promotion
fractional half = 1/2;
i32 wholeNumber = 5;
fractional exactSum = half + wholeNumber;    // Exactly 11/2 (fractional)
f32 floatNum = 2.5f;
f32 floatResult = half + floatNum;           // 3.0f (float)

// Overflow protection - throws exception if numerator or denominator exceeds i32 range
try {
    fractional huge = (i32.MAX_VALUE / 2) * (i32.MAX_VALUE / 2); // May throw OverflowException
} catch (OverflowException e) {
    System.err.println("Fractional overflow: " + e.getMessage());
}

// Explicit conversion when needed
f64 approximate = result.toF64();     // 0.8333333333333334
i32 truncated = result.toI32();       // 0 (truncates towards zero)
String display = result.toString();   // "5/6"
```

## Reference Types

### Strings and Character Arrays

BluePrint distinguishes between basic character arrays and rich string objects:

```blueprint
// Basic string type (character array)
str name = "Hello, World!";     // Character array (str == char[])
str empty = "";                 // Empty character array
str nullStr = null;             // Null reference

// Character array operations
char firstChar = name[0];       // Access individual characters
u32 length = name.length;       // Get array length

// Rich String type with methods
String message = new String("Hello");
String result = message.concat(" World");
bool isEqual = message.equals("Hello");
u32 stringLength = message.length();

// Type conversion rules
String richString = String.from("Hello");  // Convert str to String
str basicString = richString.toCharArray(); // Convert String to str
```

```blueprint
// String operations in blueprints
blueprint StringUtils {
    public concatenate(a, b) {
        input:
            a: str,
            b: str;
        output: str;
        requires: a != null && b != null;
        ensures: concatenate.length == a.length + b.length;
    }
    
    public toUpperCase(input) {
        input: input: str;
        output: str;
        requires: input != null;
        ensures: toUpperCase.length == input.length;
    }
}
```

### Arrays (Fixed-Size)

All arrays in BluePrint are fixed-size upon declaration. The type definition specifies it's an array but not the size - the size is determined at instantiation:

```blueprint
// Fixed-size array declarations - size determined at instantiation
i32[] numbers = {1, 2, 3, 4, 5};        // Array of 5 integers (fixed)
str[] names = {"Alice", "Bob", "Charlie"}; // Array of 3 strings (fixed)
f64[] measurements = new f64[10];        // Uninitialized array of 10 floats (fixed)

// Array access and manipulation
numbers[0] = 42;                         // Set first element
i32 first = numbers[0];                  // Get first element
u32 size = numbers.length;               // Get array size (always 5 for this array)

// Multi-dimensional fixed arrays
i32[][] matrix = new i32[3][4];          // 3x4 matrix (fixed dimensions)
matrix[0][0] = 1;                        // Set element

// Arrays cannot change size after creation
// numbers[5] = 6; // COMPILE ERROR - index out of bounds, array only has 5 elements
// Use dynamic collections for resizable data
```

### Dynamic Collections

For dynamic sizing, use collection classes:

```blueprint
// Dynamic collections from standard library
List<i32> dynamicNumbers = new ArrayList<i32>();
dynamicNumbers.add(42);
dynamicNumbers.add(24);
u32 count = dynamicNumbers.size();       // Dynamic size

Set<str> uniqueNames = new HashSet<str>();
Map<str, i32> nameToAge = new HashMap<str, i32>();

// Collection operations in blueprints
blueprint DynamicProcessor<T> {
    public processItems(items) {
        input: items: List<T>;
        output: List<T>;
        requires: items != null;
        ensures: processItems.size() <= items.size();
    }
}
```

### Array and Reference Operations

```blueprint
blueprint ArrayUtils<T> {
    public get(array, index) {
        input:
            array: T[],
            index: u32;
        output: T;
        requires: array != null && index < array.length;
    }
    
    public set(array, index, value) {
        input:
            array: T[],
            index: u32,
            value: T;
        output: void;
        requires: array != null && index < array.length;
        ensures: array[index] == value;
    }
}

blueprint ReferenceUtils<T> {
    public dereference(ref) {
        input: ref: T*;
        output: T;
        requires: ref != null;
    }
    
    public addressOf(value) {
        input: value: T;
        output: T*;
        ensures: addressOf != null;
        ensures: *addressOf == value;
    }
}
```

## Generic Types

### Generic Classes

```blueprint
blueprint Stack<T> {
    public push(item) {
        input: item: T;
        output: void;
        ensures: this.size() == old(this.size()) + 1;
        ensures: this.top() == item;
    }
    
    public pop() {
        output: T;
        requires: this.size() > 0;
        ensures: this.size() == old(this.size()) - 1;
    }
    
    public top() {
        output: T;
        requires: this.size() > 0;
    }
    
    public size() {
        output: u32;
        ensures: size >= 0;
    }
}

class ArrayStack<T> : Stack<T> {
    private T[] items;
    private u32 count;
    
    public ArrayStack() {
        this.items = new T[10];
        this.count = 0;
    }
    
    public void push(T item) {
        if (count >= items.length) {
            resize();
        }
        items[count++] = item;
    }
    
    public T pop() {
        return items[--count];
    }
    
    public T top() {
        return items[count - 1];
    }
    
    public u32 size() {
        return count;
    }
    
    private void resize() {
        T[] newItems = new T[items.length * 2];
        for (u32 i = 0; i < count; i++) {
            newItems[i] = items[i];
        }
        items = newItems;
    }
}
```

### Generic Methods

```blueprint
blueprint Utilities {
    public swap<T>(array, i, j) {
        input:
            array: T[],
            i: u32,
            j: u32;
        output: void;
        requires: array != null && i < array.length && j < array.length;
        ensures: array[i] == old(array[j]) && array[j] == old(array[i]);
    }
    
    public max<T>(a, b, comparator) {
        input:
            a: T,
            b: T,
            comparator: Function<T, T, i32>;
        output: T;
        requires: comparator != null;
        ensures: comparator(max, a) >= 0 && comparator(max, b) >= 0;
    }
}
```

### Bounded Generics

Generics can be constrained to implement specific blueprints using comma-separated lists. Generics are invariant in BluePrint:

```blueprint
blueprint Comparable<T> {
    public compareTo(other) {
        input: other: T;
        output: i32;
        ensures: (compareTo == 0) <==> (this.equals(other));
        ensures: (compareTo > 0) <==> (other.compareTo(this) < 0);
    }
}

blueprint Serializable {
    public serialize() {
        output: str;
        ensures: serialize != null;
    }
}

blueprint Cloneable<T> {
    public clone() {
        output: T;
        ensures: clone != null;
        ensures: !clone.equals(this); // Different reference
    }
}

// Multiple bounds using comma separation - type must satisfy ALL constraints
blueprint SortedContainer<T : Comparable<T>, Serializable, Cloneable<T>> {
    public add(item) {
        input: item: T;
        output: void
        requires: item != null;
        ensures: this.contains(item);
        ensures: forall i: 0 <= i < this.size() - 1 ==> this.get(i).compareTo(this.get(i + 1)) <= 0;
    }
    
    public serialize() {
        output: str;
        ensures: serialize != null;
    }
}

// Implementation must satisfy ALL bounds
class StringList : SortedContainer<String, Serializable, Cloneable<String>> {
    // String must implement:
    // 1. Comparable<String> (for sorting)
    // 2. Serializable (for serialization)  
    // 3. Cloneable<String> (for cloning)
    
    public void add(String item) {
        // Implementation must maintain sorted order
        // Can use item.compareTo() because String : Comparable<String>
    }
    
    public str serialize() {
        // Implementation must serialize container
        // Can use item.serialize() because String : Serializable
        return "SerializedStringList";
    }
}

// Example of constraint verification
class InvalidList : SortedContainer<i32, Serializable, Cloneable<i32>> {
    // COMPILE ERROR: i32 does not implement Serializable
    // All bounds must be satisfied by the type argument
}

// Generic variance examples showing invariance
class Example {
    public void demonstrateInvariance() {
        SortedContainer<String, Serializable, Cloneable<String>> stringContainer;
        SortedContainer<Object, Serializable, Cloneable<Object>> objectContainer;
        
        // This would be invalid - generics are invariant
        // objectContainer = stringContainer; // COMPILE ERROR
        // stringContainer = objectContainer; // COMPILE ERROR
    }
}
```

## Multiple Inheritance

### Interface Inheritance

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
        output: void;
    }
    
    public getPosition() {
        output: Point;
        ensures: getPosition != null;
    }
}

blueprint Resizable {
    public resize(factor) {
        input: factor: f64;
        output: void;
        requires: factor > 0.0;
        ensures: this.getArea() == old(this.getArea()) * factor * factor;
    }
    
    public getArea() {
        output: f64;
        ensures: getArea >= 0.0;
    }
}

// Multiple inheritance
class Shape : Drawable, Movable, Resizable {
    protected Point position;
    protected f64 width, height;
    
    public void draw() {
        // Implementation
    }
    
    public void move(f64 x, f64 y) {
        this.position = new Point(x, y);
    }
    
    public Point getPosition() {
        return this.position;
    }
    
    public void resize(f64 factor) {
        this.width *= factor;
        this.height *= factor;
    }
    
    public f64 getArea() {
        return width * height;
    }
}
```

### Diamond Problem Resolution

BluePrint uses virtual inheritance (similar to C++) to resolve diamond inheritance problems:

```blueprint
blueprint A {
    public method() {
        output: String;
        ensures: method != null;
    }
}

blueprint B : A {
    public method() {
        output: String
        ensures: method == "B";
        ensures: method != null;  // Inherited from A
    }
}

blueprint C : A {
    public method() {
        output: String
        ensures: method == "C";
        ensures: method != null;  // Inherited from A
    }
}

// Virtual inheritance - explicit casting required for access
class D : B, C {
    // Must explicitly override to resolve ambiguity
    public String method() {
        return "D";
    }
}

// Usage requires explicit casting to resolve ambiguity
class DiamondExample {
    public void demonstrateVirtualInheritance() {
        D obj = new D();
        
        // Direct access to D's method
        String directResult = obj.method();  // "D"
        
        // Explicit casting required for blueprint access
        A asA_fromB = (B) obj;  // Cast to A via B path
        A asA_fromC = (C) obj;  // Cast to A via C path
        
        // This would be ambiguous - compile error
        // A asA = obj;  // COMPILE ERROR: Ambiguous conversion from D to A
        
        // Use specific paths
        String viaB = asA_fromB.method();  // Uses B's contract path
        String viaC = asA_fromC.method();  // Uses C's contract path
    }
}
```

#### Contract Conflicts in Diamond Inheritance

When multiple inheritance paths create conflicting contract requirements, this is a **compile-time error by design**:

```blueprint
blueprint BaseCalculator {
    public calculate(value) {
        input: value: f64;
        output: f64
        ensures: calculate >= 0.0;  // Must be non-negative
    }
}

blueprint PositiveCalculator : BaseCalculator {
    public calculate(value) {
        input: value: f64;
        output: f64
        ensures: calculate > 0.0;   // Must be positive (compatible with base)
        ensures: calculate >= 0.0;  // Inherited from BaseCalculator
    }
}

blueprint NegativeCalculator : BaseCalculator {
    public calculate(value) {
        input: value: f64;
        output: f64
        ensures: calculate < 0.0;   // Must be negative (CONFLICTS with PositiveCalculator)
        ensures: calculate >= 0.0;  // Inherited from BaseCalculator
    }
}

// This will cause a compile-time error due to conflicting ensures clauses
class ConflictingCalculator : PositiveCalculator, NegativeCalculator {
    // COMPILE ERROR: Conflicting contracts
    // - PositiveCalculator.calculate ensures: calculate > 0.0
    // - NegativeCalculator.calculate ensures: calculate < 0.0
    // These cannot both be satisfied simultaneously
    
    public f64 calculate(f64 value) {
        // No implementation can satisfy both contracts
        return value;
    }
}
```

**Error Message:**
```
Error: Conflicting blueprint contracts in ConflictingCalculator.calculate()
  Conflict 1: PositiveCalculator ensures: calculate > 0.0
  Conflict 2: NegativeCalculator ensures: calculate < 0.0
  Location: ConflictingCalculator.calculate()
  Resolution: These contracts cannot be satisfied simultaneously.
  Suggestion: Use composition instead of multiple inheritance for conflicting behaviors.
```

This design choice ensures that contract violations are caught at compile time rather than causing runtime inconsistencies.

## Type Inference

### Local Variable Inference

```blueprint
class Example {
    public void demonstrateInference() {
        // Type inferred from initializer
        var number = 42;           // Inferred as i32
        var price = 19.99;         // Inferred as f64
        var name = "Alice";        // Inferred as String
        var list = new ArrayList<String>(); // Inferred as ArrayList<String>
        
        // Array inference
        var numbers = {1, 2, 3};   // Inferred as i32[]
        var mixed = {1, 2.5};      // Error: Cannot infer common type
    }
}
```

### Generic Type Inference

```blueprint
class GenericExample {
    public void demonstrateGenericInference() {
        List<String> names = new ArrayList<>();  // <> inferred as <String>
        
        // Method call inference
        String max = Utilities.max("hello", "world", String::compareTo);
        // T inferred as String from arguments
    }
}
```

## Null Safety

### Nullable Types

```blueprint
blueprint FileSystem {
    public readFile(filename) {
        input: filename: String;
        output: String?  // Nullable return type
        requires: filename != null;
        ensures: (readFile == null) ==> (!fileExists(filename));
    }
    
    public writeFile(filename, content) {
        input:
            filename: String,
            content: String;
        output: bool
        requires: filename != null && content != null;
    }
}

class FileSystemImpl : FileSystem {
    public String? readFile(String filename) {
        if (!fileExists(filename)) {
            return null;
        }
        // Read and return file content
        return content;
    }
    
    public void useFile() {
        String? content = readFile("config.txt");
        
        // Null check required before use
        if (content != null) {
            System.out.println(content.length());
        }
        
        // Or use safe navigation
        u32? length = content?.length();
    }
}
```

## Type Casting and Checking

### Safe Casting

```blueprint
class Animal { }
class Dog : Animal {
    public void bark() { }
}

class Example {
    public void demonstrateCasting() {
        Animal animal = new Dog();
        
        // Type checking
        if (animal instanceof Dog) {
            Dog dog = (Dog) animal;  // Safe cast
            dog.bark();
        }
        
        // Alternative safe cast
        Dog? dog = animal as Dog;  // Returns null if cast fails
        dog?.bark();
    }
}
```
