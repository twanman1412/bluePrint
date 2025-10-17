# Examples

This section contains practical examples of BluePrint programs demonstrating the blueprint-first development approach.

## Basic Examples

### Mathematical Calculator

```blueprint
blueprint MathCalculator {
    public add(a, b) {
        input: a: f64, b: f64;
        output: f64;
        ensures: add == a + b;
    }
    
    public subtract(a, b) {
        input: a: f64, b: f64;
        output: f64;
        ensures: subtract == a - b;
    }
}

class BasicMathCalculator : MathCalculator {
    public f64 add(f64 a, f64 b) {
        return a + b;
    }
    
    public f64 subtract(f64 a, f64 b) {
        return a - b;
    }
}

class MathCalculatorApp : System.Application {
    public static void main(str[] args) {
        BasicMathCalculator calc = new BasicMathCalculator();
        f64 result = calc.add(5.0, 3.0);
        System.out.println("5 + 3 = " + result);
    }
}
```

### Fibonacci Number Generator

```blueprint
blueprint FibonacciSequence {
    public fibonacci(n) {
        input: n: u32;
        output: u32;
        default: n == 0 ==> 0;
        default: n == 1 ==> 1;
        ensures: (n > 1) ==> (fibonacci == fibonacci(n-1) + fibonacci(n-2));
    }
}

class RecursiveFibonacciCalculator : FibonacciSequence {
    public u32 fibonacci(u32 n) {
        // Base cases handled by blueprint defaults
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

class FibonacciDemoApp : System.Application {
    public static void main(str[] args) {
        RecursiveFibonacciCalculator fibCalc = new RecursiveFibonacciCalculator();
        for (u32 i = 0; i < 10; i++) {
            System.out.println("fib(" + i + ") = " + fibCalc.fibonacci(i));
        }
    }
}
```

## User Interface Widget Example

This example demonstrates multiple inheritance with UI components that must be both drawable and resizable:

```blueprint
blueprint Drawable {
    public draw(context) {
        input: context: GraphicsContext;
        output: void;
        requires: context != null;
        ensures: this.isVisible();
    }
    
    public isVisible() {
        output: bool;
    }
}

blueprint Resizable {
    public resize(width, height) {
        input: width: u32, height: u32;
        output: void;
        requires: width > 0 && height > 0;
        ensures: this.getWidth() == width && this.getHeight() == height;
    }
    
    public getWidth() {
        output: u32;
        ensures: getWidth > 0;
    }
    
    public getHeight() {
        output: u32;
        ensures: getHeight > 0;
    }
}

blueprint Clickable {
    public onClick(event) {
        input: event: MouseEvent;
        output: void;
        requires: event != null;
    }
}

class Button : Drawable, Resizable, Clickable {
    private u32 width, height;
    private str text;
    private bool visible;
    
    public Button(str text, u32 width, u32 height) {
        this.text = text;
        this.width = width;
        this.height = height;
        this.visible = true;
    }
    
    public void draw(GraphicsContext context) {
        context.fillRect(0, 0, this.width, this.height);
        context.drawText(this.text, this.width / 2, this.height / 2);
    }
    
    public bool isVisible() {
        return this.visible;
    }
    
    public void resize(u32 width, u32 height) {
        this.width = width;
        this.height = height;
    }
    
    public u32 getWidth() {
        return this.width;
    }
    
    public u32 getHeight() {
        return this.height;
    }
    
    public void onClick(MouseEvent event) {
        System.out.println("Button clicked: " + this.text);
    }
}
```

## User Interface Widget Example

This example demonstrates multiple inheritance with UI components that must be both drawable and resizable:

```blueprint
blueprint Drawable {
    public draw(context) {
        input: context: GraphicsContext;
        output: void;
        requires: context != null;
        ensures: this.isVisible();
    }
    
    public isVisible() {
        output: bool;
    }
}

blueprint Resizable {
    public resize(width, height) {
        input: width: u32, height: u32;
        output: void;
        requires: width > 0 && height > 0;
        ensures: this.getWidth() == width && this.getHeight() == height;
    }
    
    public getWidth() {
        output: u32;
        ensures: getWidth > 0;
    }
    
    public getHeight() {
        output: u32;
        ensures: getHeight > 0;
    }
}

blueprint Clickable {
    public onClick(event) {
        input: event: MouseEvent;
        output: void;
        requires: event != null;
    }
}

class Button : Drawable, Resizable, Clickable {
    private u32 width, height;
    private str text;
    private bool visible = true;
    
    public Button(str text, u32 width, u32 height) {
        this.text = text;
        this.width = width;
        this.height = height;
    }
    
    public void draw(GraphicsContext context) {
        if (visible) {
            context.drawRectangle(0, 0, this.width, this.height);
            context.drawText(text, this.width/2, this.height/2);
        }
    }
    
    public void resize(u32 width, u32 height) {
        this.width = width;
        this.height = height;
    }
    
    public void onClick(MouseEvent event) {
        System.out.println("Button '" + this.text + "' clicked!");
    }
    
    public bool isVisible() {
        return this.visible;
    }
    
    public u32 getWidth() {
        return this.width;
    }
    
    public u32 getHeight() {
        return this.height;
    }
}

class UIApplication : System.Application {
    public static void main(str[] args) {
        Button submitButton = new Button("Submit", 100, 30);
        Button cancelButton = new Button("Cancel", 80, 30);
        
        // All buttons are drawable, resizable, and clickable
        GraphicsContext ctx = new GraphicsContext();
        submitButton.draw(ctx);
        cancelButton.resize(120, 35);
        
        // Simulate click events
        MouseEvent clickEvent = new MouseEvent(50, 15);
        submitButton.onClick(clickEvent);
        
        System.out.println("Submit button size: " + 
                          submitButton.getWidth() + "x" + submitButton.getHeight());
    }
}
```

## Text Processing Example

```blueprint
blueprint TextProcessor {
    public processText(input) {
        input: input: str;
        output: str;
        default: input.length == 0 ==> "empty input";
        requires: input != null;
        ensures: processText.length > 0;
    }
    
    public countWords(text) {
        input: text: str;
        output: u32;
        default: text.length == 0 ==> 0;
        requires: text != null;
        ensures: countWords >= 0;
    }
}

class StringTextAnalyzer : TextProcessor {
    public str processText(str input) {
        // Base case handled by blueprint default
        // Convert str (char array) to String for processing
        String text = new String(input);
        return text.toUpperCase().toCharArray(); // Convert back to str
    }
    
    public u32 countWords(str text) {
        // Base case handled by blueprint default 
        u32 count = 1;
        for (u32 i = 0; i < text.length; i++) {
            if (text[i] == ' ') {
                count++;
            }
        }
        return count;
    }
}
```

## Array Processing Example

```blueprint
blueprint NumericArrayProcessor {
    public findMax(numbers) {
        input: numbers: i32[]; // Fixed-size array
        output: i32;
        requires: numbers.length > 0;
        ensures: forall i: 0 <= i < numbers.length ==> findMax >= numbers[i];
    }
    
    public sum(values) {
        input: values: i32[];
        output: i32;
        requires: forall i: 0 <= i < values.length ==> values[i] >= 0;
        ensures: sum >= 0;
    }
}

class IntegerArrayProcessor : NumericArrayProcessor {
    public i32 findMax(i32[] numbers) {
        i32 max = numbers[0];
        for (u32 i = 1; i < numbers.length; i++) {
            if (numbers[i] > max) {
                max = numbers[i];
            }
        }
        return max;
    }
    
    public i32 sum(i32[] values) {
        i32 total = 0;
        for (u32 i = 0; i < values.length; i++) {
            total += values[i];
        }
        return total;
    }
}
```

## Student Management System Example

```blueprint
blueprint StudentRegistry {
    public addStudent(students, newStudent) {
        input: 
            students: List<Student>,
            newStudent: Student;
        output: void;
        requires: students != null && newStudent != null;
        ensures: students.contains(newStudent);
        ensures: students.size() == old(students.size()) + 1;
    }
    
    public findStudent(students, name) {
        input:
            students: List<Student>,
            name: str;
        output: Student?;
        requires: students != null && name != null;
    }
}

class StudentRecord {
    private str studentName;
    private u32 studentAge;
    
    public StudentRecord(str name, u32 age) {
        this.studentName = name;
        this.studentAge = age;
    }
    
    public str getName() {
        return studentName;
    }
    
    public u32 getAge() {
        return studentAge;
    }
}

class UniversityStudentRegistry : StudentRegistry {
    public void addStudent(List<Student> students, Student newStudent) {
        students.add(newStudent);
    }
    
    public Student? findStudent(List<Student> students, str name) {
        for (Student student : students) {
            if (student.getName() == name) {
                return student;
            }
        }
        return null;
    }
}
```

## Comprehensive Web Service Example

This example demonstrates a complete BluePrint application combining multiple language features:

See [Modules](modules.md) for import system details.

```blueprint
// Import necessary modules
import [List, Map] from bundle std.collections;
import bundle std.io;
import [HttpServer, HttpRequest, HttpResponse] from bundle network.http;

// Blueprint specifications with contracts and default values
blueprint UserService {
    public createUser(userData) {
        input: userData: UserData;
        output: User;
        throws: ValidationException, DatabaseException;
        requires: userData != null && userData.isValid();
        ensures: createUser.getId() > 0;
        ensures: this.findUser(createUser.getId()) != null;
    }
    
    public findUser(userId) {
        input: userId: u32;
        output: User?;
        throws: DatabaseException;
        requires: userId > 0;
        default: userId == 0 ==> null;
    }
    
    public updateUser(userId, updates) {
        input: userId: u32, updates: UserData;
        output: bool;
        throws: ValidationException, DatabaseException;
        requires: userId > 0 && updates != null && updates.isValid();
        default: userId == 0 ==> false;
        default: updates == null ==> false;
        ensures: updateUser ==> (this.findUser(userId).getLastModified() > old(this.findUser(userId).getLastModified()));
        ensures: !updateUser ==> (this.findUser(userId) == null);
    }
    
    public getUserCount() {
        output: u32;
        ensures: getUserCount >= 0;
    }
}

// Blueprint for HTTP server with async operations
blueprint HttpServer {
    public start(port) {
        input: port: u32;
        output: void;
        throws: ServerException;
        requires: port > 0 && port <= 65535;
        default: port == 0 ==> throw new ServerException("Invalid port");
        ensures: this.isRunning();
    }
    
    public async handleRequest(request) {
        input: request: HttpRequest;
        output: Future<HttpResponse>;
        throws: RequestException;
        requires: request != null;
        ensures: handleRequest != null;
    }
    
    public isRunning() {
        output: bool;
    }
    
    public stop() {
        output: void;
        ensures: !this.isRunning();
    }
}

// Blueprint for validatable data with constraints
blueprint Validatable {
    public isValid() {
        output: bool;
    }
    
    public validate() {
        output: void;
        throws: ValidationException;
        ensures: this.isValid();
    }
}

// Blueprint for resource management
blueprint AutoCloseable {
    public close() {
        output: void;
        ensures: this.isClosed();
    }
    
    public isClosed() {
        output: bool;
    }
}
```

See [Exception Handling](exceptions.md) for exception system details.

```blueprint
// Exception handling with blueprint contracts
class ValidationException : Exception {
    private str field;
    
    public ValidationException(str message, str field) {
        super(message);
        this.field = field;
    }
    
    public str getField() {
        return this.field;
    }
}

class DatabaseException : Exception {
    public DatabaseException(str message) {
        super(message);
    }
}

class ServerException : Exception {
    public ServerException(str message) {
        super(message);
    }
}

class RequestException : Exception {
    public RequestException(str message) {
        super(message);
    }
}
```

See [Type System](types.md) for type system details.

```blueprint
// Data classes with blueprint constraints and validation
class UserData : Validatable {
    private str name;
    private str email;
    private u32 age;
    
    public UserData(str name, str email, u32 age) {
        this.name = name;
        this.email = email;
        this.age = age;
    }
    
    public bool isValid() {
        return this.name != null && this.name.length > 0 && 
               this.email != null && this.email.contains("@") && 
               this.age >= 13 && this.age <= 150;
    }
    
    public void validate() {
        if (this.name == null || this.name.length == 0) {
            throw new ValidationException("Name cannot be empty", "name");
        }
        if (this.email == null || !this.email.contains("@")) {
            throw new ValidationException("Invalid email format", "email");
        }
        if (this.age < 13 || this.age > 150) {
            throw new ValidationException("Age must be between 13 and 150", "age");
        }
    }
    
    // Getters
    public str getName() { return this.name; }
    public str getEmail() { return this.email; }
    public u32 getAge() { return this.age; }
}

class User {
    private u32 id;
    private UserData data;
    private u64 lastModified;
    
    public User(u32 id, UserData data) {
        this.id = id;
        this.data = data;
        this.lastModified = System.currentTimeMillis();
    }
    
    public u32 getId() { return this.id; }
    public UserData getData() { return this.data; }
    public u64 getLastModified() { return this.lastModified; }
    
    public void updateData(UserData newData) {
        this.data = newData;
        this.lastModified = System.currentTimeMillis();
    }
}
```

See [Memory Management](memory.md) for resource management details.

```blueprint
// Service implementation with comprehensive blueprint contracts
class UserServiceImpl : UserService, AutoCloseable {
    private Map<u32, User> users;
    private u32 nextId = 1;
    private bool closed = false;
    
    public UserServiceImpl() {
        this.users = new HashMap<u32, User>();
    }
    
    public User createUser(UserData userData) {
        // Blueprint preconditions and defaults handle validation automatically
        // This method only runs if userData != null && userData.isValid()
        
        User user = new User(this.nextId++, userData);
        this.users.put(user.getId(), user);
        return user;
    }
    
    public User? findUser(u32 userId) {
        // Blueprint default handles userId == 0 case automatically
        // This method only runs if userId > 0
        return this.users.get(userId);
    }
    
    public bool updateUser(u32 userId, UserData updates) {
        // Blueprint defaults handle invalid inputs automatically
        // This method only runs if all preconditions are met
        
        User? user = this.findUser(userId);
        if (user != null) {
            user.updateData(updates);
            return true;
        }
        return false;
    }
    
    public u32 getUserCount() {
        return (u32)this.users.size();
    }
    
    // Resource cleanup with blueprint contracts
    public void close() {
        this.users.clear();
        this.closed = true;
    }
    
    public bool isClosed() {
        return this.closed;
    }
}
```

See [Concurrency](concurrency.md) for async/await details.

```blueprint
// Async web server with comprehensive blueprint integration
class UserWebService : HttpServer, AutoCloseable {
    private UserService userService;
    private bool running = false;
    
    public UserWebService() {
        this.userService = new UserServiceImpl();
    }
    
    public void start(u32 port) {
        // Blueprint preconditions and defaults handle invalid ports automatically
        // This method only runs if port is valid (1-65535)
        
        this.running = true;
        System.out.println("Server starting on port " + port);
    }
    
    public async Future<HttpResponse> handleRequest(HttpRequest request) {
        // Blueprint preconditions ensure request != null
        
        try {
            str path = request.getPath();
            str method = request.getMethod();
            
            if (path.equals("/users") && method.equals("POST")) {
                return await this.createUserEndpoint(request);
            } else if (path.startsWith("/users/") && method.equals("GET")) {
                u32 userId = this.parseUserId(path);
                return await this.getUserEndpoint(userId);
            } else if (path.startsWith("/users/") && method.equals("PUT")) {
                u32 userId = this.parseUserId(path);
                return await this.updateUserEndpoint(userId, request);
            } else if (path.equals("/users/count") && method.equals("GET")) {
                return await this.getUserCountEndpoint();
            }
            
            return new HttpResponse(404, "Not Found");
            
        } catch (ValidationException e) {
            return new HttpResponse(400, "Validation Error: " + e.getMessage());
        } catch (DatabaseException e) {
            return new HttpResponse(500, "Database Error: " + e.getMessage());
        } catch (Exception e) {
            return new HttpResponse(500, "Internal Server Error");
        }
    }
    
    public bool isRunning() {
        return this.running;
    }
    
    public void stop() {
        this.running = false;
    }
    
    private async Future<HttpResponse> createUserEndpoint(HttpRequest request) {
        str body = request.getBody();
        UserData userData = this.parseUserData(body);
        
        // Blueprint contracts in UserService ensure proper validation
        userData.validate(); // Explicit validation call
        User user = this.userService.createUser(userData);
        str response = this.formatUserJson(user);
        
        return new HttpResponse(201, response);
    }
    
    private async Future<HttpResponse> getUserEndpoint(u32 userId) {
        // Blueprint contracts ensure userId validation
        User? user = this.userService.findUser(userId);
        if (user != null) {
            str response = this.formatUserJson(user);
            return new HttpResponse(200, response);
        }
        return new HttpResponse(404, "User not found");
    }
    
    private async Future<HttpResponse> updateUserEndpoint(u32 userId, HttpRequest request) {
        str body = request.getBody();
        UserData updates = this.parseUserData(body);
        
        // Blueprint contracts handle validation automatically
        updates.validate(); // Explicit validation
        bool updated = this.userService.updateUser(userId, updates);
        
        if (updated) {
            User? user = this.userService.findUser(userId);
            str response = this.formatUserJson(user);
            return new HttpResponse(200, response);
        }
        return new HttpResponse(404, "User not found");
    }
    
    private async Future<HttpResponse> getUserCountEndpoint() {
        u32 count = this.userService.getUserCount();
        str response = "{\"count\":" + count + "}";
        return new HttpResponse(200, response);
    }
    
    private UserData parseUserData(str json) {
        // Simplified JSON parsing - in reality, use proper JSON library
        str name = this.extractJsonValue(json, "name");
        str email = this.extractJsonValue(json, "email");
        str ageStr = this.extractJsonValue(json, "age");
        u32 age = this.parseInteger(ageStr);
        
        return new UserData(name, email, age);
    }
    
    private str extractJsonValue(str json, str key) {
        // Simplified extraction - blueprint default could handle malformed JSON
        i32 keyIndex = json.indexOf("\"" + key + "\":");
        if (keyIndex == -1) return "";
        
        i32 valueStart = json.indexOf("\"", keyIndex + key.length + 3) + 1;
        i32 valueEnd = json.indexOf("\"", valueStart);
        
        return json.substring(valueStart, valueEnd);
    }
    
    private u32 parseInteger(str value) {
        // Blueprint default could handle invalid numbers
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            return 0;
        }
    }
    
    private u32 parseUserId(str path) {
        // Extract user ID from path like "/users/123"
        str[] parts = path.split("/");
        if (parts.length >= 3) {
            return this.parseInteger(parts[2]);
        }
        return 0;
    }
    
    private str formatUserJson(User? user) {
        if (user == null) return "{}";
        
        return "{\"id\":" + user.getId() + 
               ",\"name\":\"" + user.getData().getName() + "\"" +
               ",\"email\":\"" + user.getData().getEmail() + "\"" +
               ",\"age\":" + user.getData().getAge() + "}";
    }
    
    // Resource cleanup with blueprint contracts
    public void close() {
        this.stop();
        if (this.userService instanceof AutoCloseable) {
            ((AutoCloseable)this.userService).close();
        }
    }
    
    public bool isClosed() {
        return !this.running;
    }
}
```

See [Standard Library](stdlib.md) for System.Application details.

```blueprint
// Main application with blueprint contracts
class WebServiceApp : System.Application {
    public static void main(str[] args) {
        System.out.println("Starting BluePrint Web Service...");
        
        try (UserWebService server = new UserWebService()) {
            server.start(8080);
            System.out.println("Server started on port 8080");
            
            // Keep server running
            while (server.isRunning()) {
                Thread.sleep(1000);
            }
        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
        }
        
        System.out.println("Server shutdown complete");
    }
}
```

This comprehensive example demonstrates:
- **Blueprint contracts** with preconditions, postconditions, default values, and exception specifications
- **Default value handling** that eliminates base case checking in implementations  
- **Multiple inheritance** and interface implementation with comprehensive contracts
- **Strong typing** with custom classes and generic collections
- **Exception handling** with custom exception types and blueprint integration
- **Memory management** with RAII pattern (try-with-resources) and AutoCloseable
- **Async/await** for web service handling with blueprint contracts
- **Module imports** from standard library bundles
- **Validation patterns** using blueprint contracts to ensure data integrity
- **Practical application structure** with separation of concerns and comprehensive error handling

> **Note on Error Messages**: The error messages shown in this example are not finalized, as the language implementation does not exist yet.

The blueprint system ensures that:
1. Invalid user IDs (0 or negative) are handled by defaults before method execution
2. Null or invalid data is caught by preconditions
3. Database consistency is maintained through postconditions
4. Resource cleanup is guaranteed through AutoCloseable contracts
5. Server state is properly managed through blueprint contracts
## Complete Application Demo

```blueprint
class ArrayAndStudentDemoApp : System.Application {
    public static void main(str[] args) {
        // Create fixed-size arrays
        i32[] numbers = {1, 5, 3, 9, 2}; // Fixed at 5 elements
        i32[] positiveValues = {10, 20, 30}; // Fixed at 3 elements
        
        IntegerArrayProcessor processor = new IntegerArrayProcessor();
        
        i32 maxNumber = processor.findMax(numbers);
        i32 totalSum = processor.sum(positiveValues);
        
        System.out.println("Max: " + maxNumber);
        System.out.println("Sum: " + totalSum);
        
        // Demonstrate student management
        List<StudentRecord> students = new ArrayList<StudentRecord>();
        UniversityStudentRegistry registry = new UniversityStudentRegistry();
        
        StudentRecord alice = new StudentRecord("Alice", 20);
        StudentRecord bob = new StudentRecord("Bob", 22);
        
        registry.addStudent(students, alice);
        registry.addStudent(students, bob);
        
        StudentRecord? found = registry.findStudent(students, "Alice");
        if (found != null) {
            System.out.println("Found student: " + found.getName() + 
                             ", age " + found.getAge());
        }
        
        System.out.println("Total students: " + students.size());
    }
}
```
