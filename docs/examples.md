# Examples

This section contains practical examples of BluePrint programs.

## Basic Examples

### Calculator

```blueprint
// Simple calculator example
func add(a: number, b: number) -> number {
    return a + b
}

func main() {
    let result = add(5, 3)
    print("5 + 3 = " + result)
}
```

### Fibonacci Sequence

```blueprint
// Fibonacci sequence generator
func fibonacci(n: number) -> number {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
```

## Intermediate Examples

[More complex examples]

## Advanced Examples

[Advanced language features examples]
