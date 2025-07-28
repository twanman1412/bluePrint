# Methods

In BluePrint, methods can be declared in three ways:
1. **Blueprint-specified methods**: Declared in blueprints with contracts, implemented in classes
2. **Blueprintless methods**: Directly implemented in classes without prior specification
3. **Built-in system methods**: Using standard library blueprints

## Method Declaration Approaches

### Blueprint-First Development

```blueprint
blueprint Calculator {
    public add(a, b) {
        input: a: f64, b: f64;
        output: f64;
        ensures: add == a + b;
    }
    
    public divide(numerator, denominator) {
        input:
            numerator: f64,
            denominator: f64;
        output: f64;
        requires: denominator != 0.0;
        ensures: divide * denominator == numerator;
    }
}

class SimpleCalculator : Calculator {
    public f64 add(f64 a, f64 b) {
        return a + b;
    }
    
    public f64 divide(f64 numerator, f64 denominator) {
        return numerator / denominator;
    }
}
```

### Blueprintless Development

For simple utilities or system methods that don't need behavioral contracts:

```blueprint
class StringUtils {
    public static String reverse(String input) {
        if (input == null) return null;
        char[] chars = input.toCharArray();
        for (i32 i = 0; i < chars.length / 2; i++) {
            char temp = chars[i];
            chars[i] = chars[chars.length - 1 - i];
            chars[chars.length - 1 - i] = temp;
        }
        return new String(chars);
    }
    
    public static bool isEmpty(String str) {
        return str == null || str.length() == 0;
    }
}
```

### Using Built-in System Blueprints

```blueprint
class Application : System.Application {
    public static void main(str[] args) {
        if (args.length == 0) {
            System.out.println("No arguments provided");
            return;
        }
        
        for (str arg : args) {
            System.out.println("Argument: " + arg);
        }
    }
}

class StudentList : System.List<Student> {
    private Student[] students;
    private u32 count;
    
    public Student get(u32 index) {
        if (index >= count) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + count);
        }
        return students[index];
    }
    
    public void add(Student student) {
        if (count >= students.length) {
            resize();
        }
        students[count++] = student;
    }
    
    public u32 size() {
        return count;
    }
    
    // ... other required methods
}

class FileProcessor : System.AutoCloseable {
    private InputStream input;
    private bool closed = false;
    
    public void processFile() {
        try {
            while (true) {
                i32 data = input.read();
                if (data == -1) break;
                // Process data
            }
        } catch (IOException e) {
            throw new ProcessingException("Failed to read file", e);
        }
    }
    
    public void close() {
        if (!closed) {
            try {
                input.close();
            } catch (IOException e) {
                // Log error but don't throw
            }
            closed = true;
        }
    }
    
    public bool isClosed() {
        return closed;
    }
}
```

## Method Implementation in Classes

### Method Implementation

```blueprint
class Calculator : MathUtils {
    private f64 memory = 0.0;
    
    public f64 sqrt(f64 x) {
        // Newton's method implementation
        f64 guess = x / 2.0;
        for (i32 i = 0; i < 10; i++) {
            guess = (guess + x / guess) / 2.0;
        }
        return guess;
    }
    
    public u64 factorial(u32 n) {
        if (n == 0) return 1;
        u64 result = 1;
        for (u32 i = 1; i <= n; i++) {
            result *= i;
        }
        return result;
    }
    
    public static f64 add(f64 a, f64 b) {
        return a + b;
    }
    
    public void addToMemory(f64 value) {
        this.memory += value;
    }
}
```

## Generic Methods

### Generic Blueprint Methods

```blueprint
blueprint Collection<T> {
    public map<U>(transform) {
        input: transform: Function<T, U>;
        output: Collection<U>;
        ensures: map.size() == this.size();
    }
    
    public filter(predicate) {
        input: predicate: Function<T, bool>;
        output: Collection<T>;
        ensures: filter.size() <= this.size();
        ensures: forall item in filter ==> predicate(item);
    }
}
```

### Generic Implementation

```blueprint
class ArrayList<T> : Collection<T> {
    private T[] items;
    private u32 count;
    
    public Collection<U> map<U>(Function<T, U> transform) {
        ArrayList<U> result = new ArrayList<U>();
        for (T item : items) {
            result.add(transform(item));
        }
        return result;
    }
    
    public Collection<T> filter(Function<T, bool> predicate) {
        ArrayList<T> result = new ArrayList<T>();
        for (T item : items) {
            if (predicate(item)) {
                result.add(item);
            }
        }
        return result;
    }
}
```

## Method Types and Higher-Order Methods

### Method Type Declarations

```blueprint
// Function type aliases
typedef Predicate<T> = Function<T, bool>;
typedef Mapper<T, U> = Function<T, U>;
typedef Comparator<T> = Function<T, T, i32>;

blueprint Algorithms {
    public sort<T>(array, comparator) {
        input:
            array: T[],
            comparator: Comparator<T>;
        output: void
        requires: array != null && comparator != null;
        ensures: forall i: 0 <= i < array.length - 1 ==> 
                 comparator(array[i], array[i + 1]) <= 0;
    }
}
```

### Lambda Expressions

```blueprint
class Example {
    public void demonstrateLambdas() {
        i32[] numbers = {1, 2, 3, 4, 5};
        
        // Filter even numbers
        List<i32> evens = numbers.filter((i32 x) => x % 2 == 0);
        
        // Map to squares
        List<i32> squares = numbers.map((i32 x) => x * x);
        
        // Sort with custom comparator
        numbers.sort((i32 a, i32 b) => b - a);  // Descending order
    }
}
```

## Method Overloading

### Explicit Overload Sets in Blueprints

Use explicit overload syntax to define multiple signatures for the same logical operation:

```blueprint
blueprint NumberFormatter {
    public format {
        overload (value: i32) {
            output: String
            ensures: format != null;
        }
        
        overload (value: f64, precision: u32) {
            output: String
            requires: precision <= 10;
            ensures: format != null;
        }
        
        overload (value: bool) {
            output: String
            ensures: format == "true" || format == "false";
        }
    }
}

class DecimalFormatter : NumberFormatter {
    // Implicit mapping - compiler matches signatures automatically
    public String format(i32 value) {
        return String.valueOf(value);
    }
    
    public String format(f64 value, u32 precision) {
        return String.format("%.*f", precision, value);
    }
    
    public String format(bool value) {
        return value ? "true" : "false";
    }
}
```

### Error Handling in Overloaded Methods

Different overloads can have different error specifications:

```blueprint
blueprint DataParser {
    public parse {
        overload (text: String) {
            output: Data
            throws: ParseException;
            requires: text != null;
            ensures: parse != null;
        }
        
        overload (file: File) {
            output: Data
            throws: IOException, ParseException;
            requires: file != null && file.exists();
            ensures: parse != null;
        }
        
        overload (stream: InputStream) {
            output: Data
            throws: IOException, ParseException;
            requires: stream != null && !stream.isClosed();
            ensures: parse != null;
            ensures: stream.isClosed();
        }
    }
}
```

## Async Methods

### Async Blueprint Declaration

```blueprint
blueprint WebService {
    public async fetchData(url) {
        input: url: str;
        output: Future<str>
        requires: url != null && url.startsWith("http");
        ensures: fetchData != null;
    }
    
    public async processData(data) {
        input: data: str;
        output: Future<ProcessedData>
        requires: data != null;
        ensures: processData != null;
    }
}
```

### Async Implementation

```blueprint
class HttpService : WebService {
    public async Future<str> fetchData(str url) {
        HttpClient client = new HttpClient();
        HttpResponse response = await client.get(url);
        return response.body();
    }
    
    public async Future<ProcessedData> processData(str data) {
        // Simulate async processing
        await Thread.sleep(100);
        return new ProcessedData(data.toUpperCase());
    }
    
    public async void demonstrateUsage() {
        str data = await fetchData("https://api.example.com/data");
        ProcessedData result = await processData(data);
        System.out.println(result);
    }
}
```
