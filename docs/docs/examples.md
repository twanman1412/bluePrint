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

### Complete Application Demo

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
