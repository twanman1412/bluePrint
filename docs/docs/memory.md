# Memory Management

BluePrint provides automatic memory management through reference-counted garbage collection combined with RAII-style destructors for deterministic resource cleanup.

## Reference-Counted Garbage Collection

BluePrint uses reference counting as the primary garbage collection mechanism. The runtime automatically tracks how many references exist to each object. When the reference count reaches zero, the object is immediately scheduled for destruction.

### Destruction Process

When an object's reference count reaches zero, BluePrint follows this destruction sequence:

1. **Execute the object's destructor body** (if defined)
2. **Call destructors of all object fields** (in reverse declaration order)
3. **Deallocate the object's memory**

This ensures proper cleanup order and prevents resource leaks.

### Reference Counting

```blueprint
class Example {
    private String data;
    
    public Example(String data) {
        this.data = data;  // Reference count increases
    }
    
    public void demonstrate() {
        Example obj1 = new Example("Hello");    // Reference count: 1
        Example obj2 = obj1;                    // Reference count: 2
        obj1 = null;                           // Reference count: 1
        obj2 = null;                           // Reference count: 0
        // Object destruction sequence begins immediately:
        // 1. Execute ~Example() destructor body
        // 2. Call data.~String()
        // 3. Deallocate Example object memory
    }
}
```

## Destructors

Destructors provide deterministic cleanup for resources that require explicit management (files, network connections, etc.).

### Destructor Declaration

Destructors are declared in blueprints using the `~name` syntax:

```blueprint
blueprint FileHandler {
    public ~FileHandler() {
        // Destructor contract - cleanup specifications
        ensures: !this.isOpen();
        ensures: this.allResourcesReleased();
    }
    
    public openFile(filename) {
        input: filename: str;
        output: bool;
        requires: filename != null;
        ensures: (openFile == true) ==> isOpen();
    }
    
    public closeFile() {
        output: void;
        ensures: !isOpen();
    }
}
```

### Destructor Implementation

```blueprint
class FileManager : FileHandler {
    private FileStream fileStream;
    private bool isOpen = false;
    
    public FileManager(str filename) {
        // Constructor
        this.fileStream = new FileStream(filename);
    }
    
    public ~FileManager() {
        // Destructor implementation
        if (this.isOpen) {
            this.closeFile();
        }
        if (this.fileStream != null) {
            this.fileStream.dispose();
            this.fileStream = null;
        }
    }
    
    public bool openFile(str filename) {
        this.isOpen = this.fileStream.open(filename);
        return this.isOpen;
    }
    
    public void closeFile() {
        if (this.fileStream != null) {
            this.fileStream.close();
        }
        this.isOpen = false;
    }
}
```

### Destructor Call Scenarios

#### 1. Automatic Destruction (Garbage Collection)
```blueprint
class AutoCleanup {
    public void demonstrateAutoCleanup() {
        FileManager fm = new FileManager("data.txt");
        fm.openFile("data.txt");
        // When fm goes out of scope and no references remain,
        // destructor is called automatically by GC
    }
}
```

#### 2. Explicit Destruction
```blueprint
class ExplicitCleanup {
    public void demonstrateExplicitCleanup() {
        FileManager fm = new FileManager("data.txt");
        fm.openFile("data.txt");
        
        try {
            // Use file manager
            processFile(fm);
        } finally {
            fm.~FileManager();  // Explicit destructor call
            // Object is still accessible but resources are cleaned up
        }
    }
}
```

#### 3. RAII Pattern
```blueprint
class RAIIExample {
    public void processFiles(str[] filenames) {
        for (str filename : filenames) {
            // RAII: constructor acquires resource
            FileManager fm = new FileManager(filename);
            
            if (fm.openFile(filename)) {
                // Use the file
                processFile(fm);
            }
            
            // Destructor automatically called when fm goes out of scope
            // This ensures file is always closed, even if exceptions occur
        }
    }
}
```

## Resource Management Patterns

### Try-With-Resources Pattern

```blueprint
blueprint AutoCloseable {
    public ~AutoCloseable() {
        ensures: this.isClosed();
    }
    
    public close() {
        output: void;
        ensures: this.isClosed();
    }
}

class DatabaseConnection : AutoCloseable {
    private Connection conn;
    
    public ~DatabaseConnection() {
        close();
    }
    
    public void close() {
        if (conn != null && !conn.isClosed()) {
            conn.close();
        }
    }
    
    // Usage with automatic cleanup
    public static void queryDatabase(str sql) {
        try (DatabaseConnection db = new DatabaseConnection()) {
            ResultSet results = db.executeQuery(sql);
            processResults(results);
        } // Destructor automatically called here
    }
}
```

### Weak References

For breaking reference cycles and cache-like scenarios - see [Concurrency](concurrency.md) for thread-safe weak reference patterns:

```blueprint
class Parent {
    private List<Child> children = new ArrayList<Child>();
    
    public void addChild(Child child) {
        children.add(child);
        child.setParent(new WeakReference<Parent>(this));
    }
}

class Child {
    private WeakReference<Parent> parent;
    
    public void setParent(WeakReference<Parent> parent) {
        this.parent = parent;
    }
    
    public Parent? getParent() {
        return parent.get();  // May return null if parent was GC'd
    }
}
```

## Memory Debugging

### Object Lifecycle Tracking

```blueprint
// Compiler flag: --debug-memory
class MemoryDebugExample {
    public void demonstrateTracking() {
        FileManager fm = new FileManager("test.txt");
        // DEBUG: Object FileManager@0x1234 created
        
        fm.openFile("test.txt");
        // DEBUG: Resource acquired: file handle for "test.txt"
        
        fm = null;
        // DEBUG: Object FileManager@0x1234 eligible for GC
        
        System.gc();
        // DEBUG: Object FileManager@0x1234 destroyed, destructor called
        // DEBUG: Resource released: file handle for "test.txt"
    }
}
```

### Memory Leak Detection

The BluePrint runtime can detect common memory leak patterns:

```blueprint
class LeakDetection {
    public void demonstrateLeak() {
        List<Object> references = new ArrayList<Object>();
        
        while (true) {
            Object obj = new Object();
            references.add(obj);
            // WARNING: Potential memory leak - unbounded collection growth
        }
    }
}
```

## Best Practices

### 1. Resource Management
- Always implement destructors for classes that manage external resources
- Use RAII pattern for deterministic cleanup
- Prefer try-with-resources for temporary resource usage

### 2. Reference Management
- Avoid circular references; use weak references when needed
- Set references to null when no longer needed for earlier GC
- Use local variables when possible to limit object lifetime

### 3. Performance Considerations

> **Note**: Performance characteristics are conceptual, as the language implementation does not exist yet. Actual memory management performance will be determined during implementation.

- Object pooling for frequently created/destroyed objects
- Lazy initialization for expensive objects
- Avoid large object graphs that delay GC

### 4. Destructor Guidelines
- Keep destructors simple and fast
- Don't throw exceptions from destructors
- Don't access other objects that might have been destroyed
- Use destructors only for resource cleanup, not business logic

## Configuration

### Garbage Collection Tuning

```blueprint
// JVM-style GC configuration
// --gc-algorithm=generational|mark-sweep|copying
// --gc-heap-size=512m
// --gc-debug=true

class GCConfiguration {
    public static void configureGC() {
        // Runtime GC hints
        System.gc();                    // Suggest immediate collection
        System.getMemoryUsage();        // Get current memory statistics
        System.setGCStrategy(Strategy.GENERATIONAL);
    }
}
```

This memory management system provides both automatic convenience and manual control when needed, ensuring both ease of use and deterministic resource cleanup.
