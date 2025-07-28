# Control Flow

BluePrint provides several control flow constructs.

## Conditional Statements

### if-else

```blueprint
if (condition) {
    // code block
} else if (anotherCondition) {
    // code block
} else {
    // code block
}
```

## Loops

### while loops

```blueprint
while (condition) {
    // loop body
}
```

### do-while loops

```blueprint
do {
    // loop body (executes at least once)
} while (condition);
```

### for loops

```blueprint
for (i32 i = 0; i < 10; i++) {
    // loop body
}

// Enhanced for loop
for (i32 item : array) {
    // iterate over array
}

// Range-based for loop
for (i32 i : 0..10) {
    // iterate from 0 to 9
}
```

## Switch Statements

```blueprint
switch (value) {
    case 1:
        // code
        break;
    case 2:
    case 3:
        // code for both 2 and 3
        break;
    default:
        // default case
        break;
}
```

## Pattern Matching

BluePrint supports pattern matching for complex data structures and result types:

```blueprint
// Result type for error handling
class Result<T, E> {
    // Success case
    public static Result<T, E> ok(T value) {
        return new OkResult<T, E>(value);
    }
    
    // Error case  
    public static Result<T, E> error(E error) {
        return new ErrorResult<T, E>(error);
    }
}

class OkResult<T, E> : Result<T, E> {
    private T value;
    
    public OkResult(T value) {
        this.value = value;
    }
    
    public T getValue() {
        return value;
    }
}

class ErrorResult<T, E> : Result<T, E> {
    private E error;
    
    public ErrorResult(E error) {
        this.error = error;
    }
    
    public E getError() {
        return error;
    }
}

class FileProcessor {
    public void handleFileResult(Result<String, IOException> result) {
        match (result) {
            case OkResult(value) -> {
                System.out.println("File content: " + value);
            }
            case ErrorResult(error) -> {
                System.err.println("File error: " + error.getMessage());
            }
        }
    }
    
    // Alternative using instanceof for simpler cases
    public void handleFileResultAlt(Result<String, IOException> result) {
        if (result instanceof OkResult) {
            OkResult<String, IOException> ok = (OkResult<String, IOException>) result;
            System.out.println("Success: " + ok.getValue());
        } else if (result instanceof ErrorResult) {
            ErrorResult<String, IOException> err = (ErrorResult<String, IOException>) result;
            System.err.println("Error: " + err.getError().getMessage());
        }
    }
}
```

## Error Handling

### Try-Catch Blocks

```blueprint
class FileProcessor {
    public void processFile(str filename) {
        try {
            str content = FileSystem.readFile(filename);
            // Process content
        } catch (FileNotFoundException e) {
            System.err.println("File not found: " + filename);
        } catch (IOException e) {
            System.err.println("IO Error: " + e.getMessage());
        } finally {
            // Cleanup code
        }
    }
}
```

### Blueprint Contracts in Control Flow

Blueprints can specify behavior within loops and conditionals:

```blueprint
blueprint LoopProcessor {
    public processItems(items) {
        input: items: i32[];
        output: i32[]
        requires: items != null;
        ensures: processItems.length == items.length;
        ensures: forall i: 0 <= i < items.length ==> processItems[i] > items[i];
    }
}

class ArrayProcessor : LoopProcessor {
    public i32[] processItems(i32[] items) {
        i32[] result = new i32[items.length];
        
        for (i32 i = 0; i < items.length; i++) {
            result[i] = items[i] + 1;  // Ensures postcondition
        }
        
        return result;
    }
}
```
