# Exception Handling

BluePrint provides a comprehensive exception handling system with a two-tier hierarchy that distinguishes between general program errors and contract violations.

## Standard Exception Hierarchy

BluePrint provides a comprehensive set of built-in exception types for common error conditions:

### Core Exception Types

```
Exception (base type)
├── IOException
│   ├── FileNotFoundException
│   ├── NetworkException
│   └── TimeoutException
├── ArithmeticException
│   ├── DivisionByZeroException
│   ├── FractionOverflowException
│   └── FractionDivisionByZeroException
├── AccessException
│   ├── NullPointerException
│   ├── IndexOutOfBoundsException
│   └── SecurityException
├── ParseException
├── IllegalArgumentException
└── BluePrintException (contract violations)
```

### Exception Usage Guidelines

#### 1. Arithmetic Exceptions
```blueprint
blueprint SafeMath {
    public divide(a, b) {
        input: a: f64, b: f64;
        output: f64
        throws: DivisionByZeroException;
        requires: b != 0.0;
        ensures: divide == a / b;
    }
    
    public createFraction(num, den) {
        input: num: i32, den: i32;
        output: fractional
        throws: FractionDivisionByZeroException, FractionOverflowException;
        requires: den != 0;
        ensures: createFraction == num / den;
    }
}
```

#### 2. Access Exceptions
```blueprint
blueprint SafeArrayAccess {
    public get(array, index) {
        input: array: i32[], index: u32;
        output: i32
        throws: NullPointerException, IndexOutOfBoundsException;
        requires: array != null && index < array.length;
        ensures: get == array[index];
    }
}
```

### Common Exception Scenarios

You mentioned these core exceptions - here are some additional ones to consider:

**Memory and Resource Management:**
- `OutOfMemoryException` - When heap space is exhausted
- `StackOverflowException` - When call stack limit is reached
- `ResourceLeakException` - When resources aren't properly released

**Type and Casting:**
- `ClassCastException` - Invalid type casting
- `IllegalStateException` - Object in invalid state for operation

**Concurrency:**
- `InterruptedException` - Thread interruption
- `DeadlockException` - Deadlock detection
- `ConcurrentModificationException` - Concurrent access violations

**System and I/O:**
- `TimeoutException` - Operation timeout
- `SecurityException` - Security/permission violations
- `ConfigurationException` - Invalid configuration

Would you like me to add any of these additional exception types to provide more comprehensive error handling coverage?

- **Exception**: Base exception type for all general exceptions (business logic, I/O errors, etc.)
- **BluePrintException**: Child of Exception, thrown automatically when blueprint contracts are violated at runtime

### Exception Types

#### 1. General Exceptions
Used for business logic, I/O operations, and other standard error conditions:

```blueprint
// Standard exception types
class IOException : Exception { }
class ParseException : Exception { }
class NetworkException : Exception { }
class FileNotFoundException : IOException { }

// Arithmetic exceptions
class DivisionByZeroException : Exception { }
class FractionOverflowException : Exception { }
class FractionDivisionByZeroException : Exception { }

// Memory and access exceptions
class NullPointerException : Exception { }
class IndexOutOfBoundsException : Exception { }

// Usage in try-catch blocks
class DataProcessor {
    public str processFile(str filename) {
        try {
            str content = FileSystem.read(filename);
            return parseData(content);
        } catch (FileNotFoundException e) {
            System.err.println("File not found: " + filename);
            return "";
        } catch (IOException e) {
            System.err.println("Failed to read file: " + e.getMessage());
            throw e;
        } catch (ParseException e) {
            System.err.println("Invalid data format: " + e.getMessage());
            throw e;
        }
    }
    
    public f64 safeDivide(f64 a, f64 b) {
        try {
            if (b == 0.0) {
                throw new DivisionByZeroException("Cannot divide " + a + " by zero");
            }
            return a / b;
        } catch (DivisionByZeroException e) {
            System.err.println("Division error: " + e.getMessage());
            return 0.0;
        }
    }
    
    public i32 safeArrayAccess(i32[] array, u32 index) {
        try {
            if (array == null) {
                throw new NullPointerException("Array is null");
            }
            if (index >= array.length) {
                throw new IndexOutOfBoundsException("Index " + index + " out of bounds for array length " + array.length);
            }
            return array[index];
        } catch (NullPointerException | IndexOutOfBoundsException e) {
            System.err.println("Access error: " + e.getMessage());
            return -1;
        }
    }
    
    public fractional safeFractionalOperation(i32 num, i32 den) {
        try {
            if (den == 0) {
                throw new FractionDivisionByZeroException("Fraction denominator cannot be zero");
            }
            
            fractional result = num / den;
            // Check for potential overflow in subsequent operations
            if (willOverflow(result)) {
                throw new FractionOverflowException("Fractional operation would cause overflow");
            }
            
            return result;
        } catch (FractionDivisionByZeroException | FractionOverflowException e) {
            System.err.println("Fractional arithmetic error: " + e.getMessage());
            return 0/1; // Return zero fraction
        }
    }
}
```

#### 2. BluePrintException (Contract Violations)
Thrown automatically by the runtime when contracts are violated:

```blueprint
blueprint SafeCalculator {
    public divide(a, b) {
        input: a: f64, b: f64;
        output: f64
        requires: b != 0.0;  // Precondition
        ensures: divide * b == a;  // Postcondition
    }
}

class Calculator : SafeCalculator {
    public f64 divide(f64 a, f64 b) {
        return a / b;
    }
    
    public void demonstrateContractViolation() {
        try {
            f64 result = divide(10.0, 0.0);  // Violates precondition
        } catch (BluePrintException e) {
            // Automatically thrown: "Precondition violated: b != 0.0"
            System.err.println("Contract violation: " + e.getMessage());
        }
    }
}
```

## Exception Specifications in Blueprints

### Throws Clause
Blueprints can specify which exceptions methods may throw:

```blueprint
blueprint FileManager {
    public readFile(filename) {
        input: filename: str;
        output: str
        throws: IOException, FileNotFoundException;
        requires: filename != null;
        ensures: readFile != null;
    }
    
    public writeFile(filename, content) {
        input: filename: str, content: str;
        output: void
        throws: IOException, SecurityException;
        requires: filename != null && content != null;
        ensures: fileExists(filename);
    }
}
```

### Exceptional Postconditions
Specify what must be true when specific exceptions are thrown:

```blueprint
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

blueprint DatabaseManager {
    public updateRecord(id, data) {
        input: id: i32, data: str;
        output: bool
        throws: DatabaseException, NetworkException;
        requires: id > 0 && data != null;
        ensures: (updateRecord == true) ==> (recordExists(id) && recordData(id) == data);
        ensures: (throws DatabaseException) ==> (database.isConsistent());
        ensures: (throws NetworkException) ==> (old(recordData(id)) == recordData(id));
    }
}
```

## Try-Catch-Finally Blocks

### Basic Exception Handling

```blueprint
class FileProcessor {
    public str processFile(str filename) {
        try {
            str content = FileSystem.read(filename);
            str processed = transformData(content);
            FileSystem.write(filename + ".processed", processed);
            return processed;
        } catch (IOException e) {
            System.err.println("IO error: " + e.getMessage());
            return null;
        } catch (TransformException e) {
            System.err.println("Transform error: " + e.getMessage());
            return content; // Return original content
        } finally {
            // Cleanup code - always executed
            cleanup();
        }
    }
}
```

### Multiple Catch Blocks

```blueprint
class NetworkClient {
    public str fetchData(str url) {
        try {
            Connection conn = openConnection(url);
            return conn.getData();
        } catch (ConnectException e) {
            System.err.println("Failed to connect: " + e.getMessage());
            return getCachedData(url);
        } catch (TimeoutException e) {
            System.err.println("Request timed out: " + e.getMessage());
            return retryWithTimeout(url, 30);
        } catch (NetworkException e) {
            System.err.println("Network error: " + e.getMessage());
            throw e; // Re-throw for caller to handle
        } finally {
            closeConnection();
        }
    }
}
```

### Try-With-Resources

For automatic resource management:

```blueprint
class DatabaseExample {
    public List<str> queryDatabase(str sql) {
        try (DatabaseConnection conn = new DatabaseConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {
            
            ResultSet results = stmt.executeQuery();
            List<str> data = new ArrayList<str>();
            
            while (results.next()) {
                data.add(results.getString(1));
            }
            
            return data;
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return new ArrayList<str>();
        }
        // Resources automatically closed here
    }
}
```

## Contract Violation Details

### Runtime Contract Checking

When contracts cannot be verified at compile time, they are checked at runtime:

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
        // Runtime contract checking order:
        // 1. Check preconditions: filename != null
        // 2. Check preconditions: fileExists(filename)
        // 3. Execute method implementation
        // 4. Check postconditions: readFile != null
        // 5. If any check fails, throw BluePrintException
        
        return FileSystem.readAllText(filename);
    }
}
```

### Contract Violation Examples

```blueprint
class ContractDemo {
    public void demonstrateViolations() {
        SafeFileReader reader = new SafeFileReader();
        
        // Precondition violation
        try {
            str content = reader.readFile(null);
        } catch (BluePrintException e) {
            // "Precondition violated: filename != null"
            System.err.println(e.getMessage());
        }
        
        // Precondition violation
        try {
            str content = reader.readFile("nonexistent.txt");
        } catch (BluePrintException e) {
            // "Precondition violated: fileExists(filename)"
            System.err.println(e.getMessage());
        }
    }
}
```

## Error Propagation

### Exception Chaining

```blueprint
class ServiceLayer {
    public str processRequest(str request) {
        try {
            return dataLayer.getData(request);
        } catch (DatabaseException e) {
            // Chain exceptions to preserve stack trace
            throw new ServiceException("Failed to process request", e);
        }
    }
}

class ServiceException : Exception {
    private Exception cause;
    
    public ServiceException(str message, Exception cause) {
        super(message);
        this.cause = cause;
    }
    
    public Exception getCause() {
        return cause;
    }
}
```

### Exception Translation

```blueprint
blueprint DataAccessLayer {
    public getData(query) {
        input: query: str;
        output: str
        throws: DataException;  // Abstracted exception type
        requires: query != null;
    }
}

class DatabaseDataAccess : DataAccessLayer {
    public str getData(str query) {
        try {
            return database.executeQuery(query);
        } catch (SQLException e) {
            // Translate database-specific exception to layer-specific exception
            throw new DataException("Database query failed: " + query, e);
        } catch (ConnectionException e) {
            throw new DataException("Database connection failed", e);
        }
    }
}
```

## Best Practices

### 1. Exception Hierarchy Design
- Create specific exception types for different error categories
- Use inheritance to group related exceptions
- Include meaningful error messages and context

### 2. Blueprint Exception Specifications
- Always specify `throws` clauses in blueprints for checked exceptions
- Use exceptional postconditions to specify cleanup guarantees
- Don't catch BluePrintException unless absolutely necessary

### 3. Error Handling Strategies
- Handle exceptions at the appropriate level of abstraction
- Use try-with-resources for automatic resource cleanup
- Log exceptions with sufficient detail for debugging
- Fail fast - don't hide or suppress exceptions without good reason

### 4. Contract Design
- Design contracts that are verifiable and meaningful
- Use preconditions to validate inputs early
- Use postconditions to guarantee method behavior
- Consider exceptional cases in contract design

## Debugging and Diagnostics

### Stack Traces

BluePrint provides detailed stack traces similar to Java:

```
BluePrintException: Precondition violated: amount > 0.0
  Contract: BankAccount.withdraw(amount)
  Violation: amount = -100.0
  at BankAccount.withdraw(bank.bp:45)
  at ATM.processWithdrawal(atm.bp:23)
  at ATM.handleTransaction(atm.bp:15)
  at Application.main(app.bp:8)
```

### Contract Debugging

Enable detailed contract verification logging:

```bash
# Compile with contract debugging
blueprintc --debug-contracts --verbose-exceptions app.bp

# Runtime output:
# [CONTRACT] Checking precondition: amount > 0.0
# [CONTRACT] ✗ FAILED: amount = -100.0
# [CONTRACT] Throwing BluePrintException
```

### Exception Analysis

```blueprint
class DebugHelper {
    public static void analyzeException(Exception e) {
        System.err.println("Exception type: " + e.getClass().getName());
        System.err.println("Message: " + e.getMessage());
        
        if (e instanceof BluePrintException) {
            BluePrintException bpe = (BluePrintException) e;
            System.err.println("Contract: " + bpe.getContractName());
            System.err.println("Violation details: " + bpe.getViolationDetails());
        }
        
        // Print full stack trace
        e.printStackTrace();
    }
}
```

This exception system provides both robust error handling for business logic and automatic contract verification, ensuring program reliability while maintaining clear separation of concerns.
