# Modules

BluePrint's module system allows you to organize code into reusable components using two import approaches: direct blueprint imports and bundle-based imports.

## Import System

### Direct Blueprint Import

Import specific blueprints directly from their file path:

```blueprint
// Direct import syntax
import blueprint_path/blueprint_name;

// Examples
import io/FileSystem;           // Import FileSystem blueprint from io directory
import math/Calculator;         // Import Calculator blueprint from math directory
import collections/List;       // Import List blueprint from collections directory

// Using imported blueprints
class MyFileManager : FileSystem {
    public str readFile(str filename) {
        // Implementation here
    }
}

class MyCalculator : Calculator {
    public f64 add(f64 a, f64 b) {
        return a + b;
    }
}
```

### Bundle-Based Import

Import entire bundles or specific blueprints from bundles (similar to Python packages):

```blueprint
// Import entire bundle
import bundle blueprint_bundle_path;

// Import specific blueprints from bundle
import [blueprint_name1, blueprint_name2] from bundle blueprint_bundle_path;

// Examples
import bundle std/collections;  // Import all blueprints from collections bundle
import bundle network/http;     // Import all HTTP-related blueprints

// Selective imports from bundles
import [List, Set, Map] from bundle std/collections;
import [HttpServer, HttpClient] from bundle network/http;
import [Calculator, MathUtils] from bundle math/advanced;
```

## Bundle Structure

Bundles are directory structures containing multiple related blueprints:

```
std/
├── collections/
│   ├── List.blueprint
│   ├── Set.blueprint
│   ├── Map.blueprint
│   └── bundle.manifest
├── io/
│   ├── FileSystem.blueprint
│   ├── InputStream.blueprint
│   ├── OutputStream.blueprint
│   └── bundle.manifest
└── math/
    ├── Calculator.blueprint
    ├── MathUtils.blueprint
    └── bundle.manifest
```

### Bundle Manifest

Each bundle contains a `bundle.manifest` file that defines which blueprints are exported:

```manifest
# bundle.manifest for std/collections
bundle: std.collections
version: 1.0.0
exports:
  - List
  - Set  
  - Map
  - Stack
  - Queue

dependencies:
  - std.core >= 1.0.0
```

## Usage Examples

### Example 1: File Processing Application

```blueprint
// Import necessary blueprints
import io/FileSystem;
import [List, Set] from bundle std/collections;
import text/StringProcessor;

class FileProcessor : FileSystem, StringProcessor {
    public void processFiles(str[] filenames) {
        List<str> results = new ArrayList<str>();
        Set<str> uniqueWords = new HashSet<str>();
        
        for (str filename : filenames) {
            str content = readFile(filename);
            str processed = processText(content);
            results.add(processed);
            
            // Extract unique words
            str[] words = processed.split(" ");
            for (str word : words) {
                uniqueWords.add(word);
            }
        }
        
        System.out.println("Processed " + results.size() + " files");
        System.out.println("Found " + uniqueWords.size() + " unique words");
    }
    
    public str readFile(str filename) {
        // Implementation
        return "";
    }
    
    public str processText(str input) {
        // Implementation
        return input.toUpperCase();
    }
}
```

### Example 2: Web Server Application

```blueprint
// Import web-related bundles
import bundle network/http;
import bundle std/collections;
import concurrent/AsyncProcessor;

class WebApplication : HttpServer, AsyncProcessor {
    private Map<str, str> routes;
    
    public WebApplication() {
        this.routes = new HashMap<str, str>();
        setupRoutes();
    }
    
    public async Future<HttpResponse> handleRequest(HttpRequest request) {
        str route = request.getPath();
        if (routes.containsKey(route)) {
            str response = routes.get(route);
            return new HttpResponse(200, response);
        }
        return new HttpResponse(404, "Not Found");
    }
    
    private void setupRoutes() {
        routes.put("/", "Welcome to BluePrint Web Server");
        routes.put("/about", "BluePrint Language Server");
    }
}
```

## Import Resolution Rules

### Search Path

BluePrint searches for modules in the following order:

1. **Current project directory** - Local blueprints and bundles
2. **Project dependencies** - Dependencies specified in project configuration
3. **Standard library** - Built-in BluePrint standard library
4. **System-wide packages** - Globally installed packages

### Namespace Resolution

```blueprint
// When importing from bundles, blueprints are available directly
import [List, Map] from bundle std/collections;

class Example {
    public void demonstrateCollections() {
        List<i32> numbers = new ArrayList<i32>();  // Direct usage
        Map<str, i32> mapping = new HashMap<str, i32>();
    }
}

// When importing entire bundles, use bundle prefix if conflicts exist
import bundle std/collections;
import bundle custom/collections;  // Potential naming conflict

class Example {
    public void demonstrateCollections() {
        // Explicit namespace resolution needed for conflicts
        std.collections.List<i32> standardList = new std.collections.ArrayList<i32>();
        custom.collections.List<i32> customList = new custom.collections.CustomList<i32>();
    }
}
```

### Error Handling

```blueprint
// Import errors are compile-time errors
import nonexistent/Blueprint;  // COMPILE ERROR: Blueprint not found
import [NonExistent] from bundle std/collections;  // COMPILE ERROR: Blueprint not exported

// Circular import detection
// If A imports B and B imports A, compile error occurs
```

## Standard Library Organization

The BluePrint standard library is organized into logical bundles:

### Core Bundles

- **std/core** - Basic types, System.Object, System.Application
- **std/collections** - List, Set, Map, Stack, Queue implementations  
- **std/io** - File I/O, streams, readers, writers
- **std/text** - String processing, regular expressions
- **std/math** - Mathematical functions, random numbers
- **std/concurrent** - Threading, async/await, channels
- **std/network** - HTTP client/server, TCP/UDP sockets
- **std/time** - Date, time, duration utilities

### Import Examples for Standard Library

```blueprint
// Most common imports
import bundle std/core;        // Always available by default
import [List, Map] from bundle std/collections;
import io/FileSystem from bundle std/io;
import [HttpServer] from bundle std/network;

// Application using standard library
class StandardApp : System.Application {
    public static void main(str[] args) {
        List<str> arguments = new ArrayList<str>();
        for (str arg : args) {
            arguments.add(arg);
        }
        
        System.out.println("Received " + arguments.size() + " arguments");
    }
}
```

This module system provides clear organization while maintaining simplicity and avoiding the complexity of package managers until needed for larger projects.
