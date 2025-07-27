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
| `fractional` | Arbitrary precision fraction (numerator/denominator) |

```blueprint
// Fractional arithmetic maintains exact precision
fractional half = 1/2;
fractional third = 1/3;
fractional result = half + third;  // Exactly 5/6, not 0.833333...

// Only loses precision when converting to floating point
f64 approximate = result.toF64();  // 0.8333333333333334

// Operations with integers and other fractionals remain exact
fractional doubled = result * 2;   // Exactly 5/3
fractional withInt = result + 1;   // Exactly 11/6
```

## Reference Types

### Strings

```blueprint
// Basic string type (character array)
str name = "Hello, World!";
str empty = "";
str nullStr = null;

// Compound String type with methods
String message = new String("Hello");
String result = message.concat(" World");
bool isEqual = message.equals("Hello");
u32 length = message.length();

// String operations in blueprints
blueprint StringUtils {
    public concat(a, b) {
        input:
            a: str,
            b: str;
        output: str
        requires: a != null && b != null;
        ensures: concat.length == a.length + b.length;
    }
}
```

### Arrays and Pointers

```blueprint
// Fixed-size arrays
i32[5] numbers = {1, 2, 3, 4, 5};
str[3] names = {"Alice", "Bob", "Charlie"};

// Dynamic arrays
i32[] dynamicNumbers = new i32[10];
str[] dynamicNames = new str[]{};

// Pointers
i32* numberPtr = &someInteger;  // & gets address
str* stringPtr = &someString;   // & gets address  
void* genericPtr = null;

// Pointer arithmetic and dereferencing
i32 value = *numberPtr;        // * dereferences pointer
numberPtr++;                   // Pointer arithmetic
i32* nextPtr = numberPtr + 1;  // Pointer offset
```

### Array and Pointer Operations

```blueprint
blueprint ArrayUtils<T> {
    public get(array, index) {
        input:
            array: T[],
            index: u32;
        output: T
        requires: array != null && index < array.length;
    }
    
    public set(array, index, value) {
        input:
            array: T[],
            index: u32,
            value: T;
        output: void
        requires: array != null && index < array.length;
        ensures: array[index] == value;
    }
}

blueprint PointerUtils<T> {
    public dereference(ptr) {
        input: ptr: T*;
        output: T
        requires: ptr != null;
    }
    
    public addressOf(value) {
        input: value: T;
        output: T*
        ensures: addressOf != null;
        ensures: *addressOf == value;  // * dereferences, & would get address
    }
}
```

## Generic Types

### Generic Classes

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
    }
    
    public top() {
        output: T
        requires: size() > 0;
    }
    
    public size() {
        output: u32
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
        output: void
        requires: array != null && i < array.length && j < array.length;
        ensures: array[i] == old(array[j]) && array[j] == old(array[i]);
    }
    
    public max<T>(a, b, comparator) {
        input:
            a: T,
            b: T,
            comparator: Function<T, T, i32>;
        output: T
        requires: comparator != null;
        ensures: comparator(max, a) >= 0 && comparator(max, b) >= 0;
    }
}
```

### Bounded Generics

```blueprint
blueprint Comparable<T> {
    public compareTo(other) {
        input: other: T;
        output: i32
        ensures: (compareTo == 0) <==> (this.equals(other));
        ensures: (compareTo > 0) <==> (other.compareTo(this) < 0);
    }
}

blueprint SortedContainer<T : Comparable<T>> {
    public add(item) {
        input: item: T;
        output: void
        ensures: contains(item);
        ensures: forall i: 0 <= i < size() - 1 ==> get(i).compareTo(get(i + 1)) <= 0;
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
        output: void
    }
    
    public getPosition() {
        output: Point
        ensures: getPosition != null;
    }
}

blueprint Resizable {
    public resize(factor) {
        input: factor: f64;
        output: void
        requires: factor > 0.0;
        ensures: getArea() == old(getArea()) * factor * factor;
    }
    
    public getArea() {
        output: f64
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

```blueprint
blueprint A {
    public method() {
        output: String
    }
}

blueprint B : A {
    public method() {
        output: String
        ensures: method == "B";
    }
}

blueprint C : A {
    public method() {
        output: String
        ensures: method == "C";
    }
}

// Explicit resolution required
class D : B, C {
    // Must explicitly override to resolve ambiguity
    public String method() {
        return "D: " + B.method() + " + " + C.method();
    }
}
```

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
