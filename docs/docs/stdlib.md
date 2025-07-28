# Standard Library

The BluePrint standard library provides essential functionality through a blueprint-first approach. All standard library components are defined as blueprints that can be implemented by user classes or used directly with built-in implementations.

## Core System Blueprints

### System.Application
The main entry point for BluePrint applications:

```blueprint
blueprint System.Application {
    public static main(args) {
        input: args: str[];
        output: void;
    }
}

class MyApp : System.Application {
    public static void main(str[] args) {
        System.out.println("Hello, BluePrint!");
    }
}
```

### System.Object
Base blueprint for all objects:

```blueprint
blueprint System.Object {
    public toString() {
        output: str;
        ensures: toString != null;
    }
    
    public equals(other) {
        input: other: Object;
        output: bool;
        ensures: equals ==> (other != null);
    }
    
    public hashCode() {
        output: i32;
    }
}
```

## Collections Framework

### List<T>
Dynamic array interface:

```blueprint
blueprint System.List<T> {
    public add(item) {
        input: item: T;
        output: void;
        ensures: this.contains(item);
        ensures: this.size() == old(this.size()) + 1;
    }
    
    public get(index) {
        input: index: u32;
        output: T;
        requires: index < this.size();
    }
    
    public size() {
        output: u32;
        ensures: size >= 0;
    }
    
    public contains(item) {
        input: item: T;
        output: bool;
    }
}
```

### Set<T>
Unique element collection:

```blueprint
blueprint System.Set<T> {
    public add(item) {
        input: item: T;
        output: bool;
        ensures: this.contains(item);
        ensures: add ==> (this.size() == old(this.size()) + 1);
        ensures: !add ==> (this.size() == old(this.size()));
    }
    
    public remove(item) {
        input: item: T;
        output: bool;
        ensures: !this.contains(item);
        ensures: remove ==> (this.size() == old(this.size()) - 1);
    }
    
    public size() {
        output: u32;
        ensures: size >= 0;
    }
}
```

### Map<K, V>
Key-value mapping:

```blueprint
blueprint System.Map<K, V> {
    public put(key, value) {
        input: key: K, value: V;
        output: V?;
        requires: key != null;
        ensures: this.get(key) == value;
    }
    
    public get(key) {
        input: key: K;
        output: V?;
        requires: key != null;
    }
    
    public containsKey(key) {
        input: key: K;
        output: bool;
        requires: key != null;
    }
}
```

## I/O System

### FileSystem
File operations:

```blueprint
blueprint System.FileSystem {
    public readFile(filename) {
        input: filename: str;
        output: str;
        throws: IOException, FileNotFoundException;
        requires: filename != null;
        ensures: readFile != null;
    }
    
    public writeFile(filename, content) {
        input: filename: str, content: str;
        output: void;
        throws: IOException;
        requires: filename != null && content != null;
        ensures: this.fileExists(filename);
    }
    
    public fileExists(filename) {
        input: filename: str;
        output: bool;
        requires: filename != null;
    }
}
```

### AutoCloseable
Resource management:

```blueprint
blueprint System.AutoCloseable {
    public close() {
        output: void;
        ensures: this.isClosed();
    }
    
    public isClosed() {
        output: bool;
    }
}
```

## Mathematical Operations

### Math
Mathematical utilities:

```blueprint
blueprint System.Math {
    public static sqrt(x) {
        input: x: f64;
        output: f64;
        requires: x >= 0.0;
        ensures: sqrt * sqrt >= x - 0.001 && sqrt * sqrt <= x + 0.001;
    }
    
    public static abs(x) {
        input: x: f64;
        output: f64;
        ensures: abs >= 0.0;
        ensures: (x >= 0.0) ==> (abs == x);
        ensures: (x < 0.0) ==> (abs == -x);
    }
    
    public static max(a, b) {
        input: a: f64, b: f64;
        output: f64;
        ensures: max >= a && max >= b;
        ensures: max == a || max == b;
    }
}
```

## String Processing

### String
Rich string type:

```blueprint
blueprint System.String {
    public length() {
        output: u32;
        ensures: length >= 0;
    }
    
    public charAt(index) {
        input: index: u32;
        output: char;
        requires: index < this.length();
    }
    
    public substring(start, end) {
        input: start: u32, end: u32;
        output: String;
        requires: start <= end && end <= this.length();
        ensures: end - start >= 0;
    }
    
    public concat(other) {
        input: other: String;
        output: String;
        requires: other != null;
        ensures: concat.length() == this.length() + other.length();
    }
    
    public equals(other) {
        input: other: String;
        output: bool;
    }
    
    public toUpperCase() {
        output: String;
        ensures: toUpperCase.length() == this.length();
    }
    
    public toLowerCase() {
        output: String;
        ensures: toLowerCase.length() == this.length();
    }
}
```

## Usage Examples

### Using Standard Library Blueprints

```blueprint
// Import standard library components
import [List, Map] from bundle std/collections;
import System.FileSystem from bundle std/io;

class DataProcessor : FileSystem {
    public void processDataFile(str filename) {
        try {
            str content = readFile(filename);
            List<str> lines = parseLines(content);
            Map<str, i32> wordCount = countWords(lines);
            displayResults(wordCount);
        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
        }
    }
    
    // Implement the FileSystem blueprint
    public str readFile(str filename) {
        // Implementation using underlying file system
        return NativeFileSystem.read(filename);
    }
    
    public void writeFile(str filename, str content) {
        NativeFileSystem.write(filename, content);
    }
    
    public bool fileExists(str filename) {
        return NativeFileSystem.exists(filename);
    }
}
```

This blueprint-first approach ensures that all standard library functionality has well-defined contracts and can be safely extended or replaced by user implementations.
