# Concurrency

BluePrint provides a comprehensive concurrency system built around async/await with channels as the primary concurrency method. The system integrates channels and blueprint contracts for safe concurrent programming.

## Overview

The concurrency system in BluePrint is built around these key components:

1. **Async/await** - Primary method for asynchronous programming with channels and futures
2. **Channels** - Type-safe message passing between concurrent contexts (integrated with async/await)
3. **Threading** - Both green threads and OS threads available as different base classes
4. **Synchronized methods** - Built into both method signatures and blueprint contracts

All concurrency constructs integrate seamlessly with BluePrint's blueprint system, allowing for compile-time verification of thread safety and concurrency contracts.

## Concurrency Models

### 1. Async/Await with Channels (Primary Concurrency Method)

BluePrint's primary concurrency approach combines async/await with channels for safe, efficient concurrent programming:

```blueprint
blueprint AsyncProcessor {
    public async processWithChannels(inputChannel, outputChannel) {
        input: 
            inputChannel: Channel<TaskData>,
            outputChannel: Channel<Result>;
        output: Future<void>
        requires: inputChannel != null && outputChannel != null;
        ensures: !inputChannel.isClosed() ==> outputChannel.itemsProcessed();
    }
    
    public async batchProcess(items) {
        input: items: TaskData[];
        output: Future<Result[]>
        requires: items != null;
        ensures: batchProcess.length == items.length;
    }
}

class ConcurrentProcessor : AsyncProcessor {
    public async Future<void> processWithChannels(Channel<TaskData> inputChannel, 
                                                 Channel<Result> outputChannel) {
        while (!inputChannel.isClosed()) {
            TaskData? data = await inputChannel.receiveAsync();
            if (data != null) {
                Result result = await processData(data);
                await outputChannel.sendAsync(result);
            }
        }
    }
    
    public async Future<Result[]> batchProcess(TaskData[] items) {
        Channel<TaskData> input = new Channel<TaskData>(capacity: 100);
        Channel<Result> output = new Channel<Result>(capacity: 100);
        
        // Start processor
        Future<void> processor = processWithChannels(input, output);
        
        // Send all items
        for (TaskData item : items) {
            await input.sendAsync(item);
        }
        input.close();
        
        // Collect results
        Result[] results = new Result[items.length];
        for (i32 i = 0; i < items.length; i++) {
            results[i] = await output.receiveAsync();
        }
        
        await processor; // Wait for processor to finish
        return results;
    }
    
    private async Future<Result> processData(TaskData data) {
        // Simulate async processing
        return await ExternalService.processAsync(data);
    }
}
```

### 2. Threading: Green Threads vs OS Threads

BluePrint provides both green threads and OS threads as different base classes, allowing developers to choose the appropriate threading model:

```blueprint
// Base threading contracts
blueprint ThreadSafeCounter {
    invariant: count >= 0;
    invariant: isThreadSafe();
    
    public synchronized increment() {
        output: void
        ensures: count == old(count) + 1;
    }
    
    public synchronized decrement() {
        output: void
        requires: count > 0;
        ensures: count == old(count) - 1;
    }
    
    public get() {
        output: i32
        ensures: get >= 0;
    }
}

// Green thread base class - lightweight, cooperative
class GreenThread {
    protected GreenThreadExecutor executor;
    
    public GreenThread() {
        this.executor = GreenThreadExecutor.current();
    }
    
    public void start() {
        executor.schedule(this::run);
    }
    
    public void yield() {
        executor.yield();
    }
    
    protected abstract void run();
}

// OS thread base class - preemptive, system threads  
class OSThread {
    protected Thread osThread;
    
    public OSThread() {
        this.osThread = new Thread(this::run);
    }
    
    public void start() {
        osThread.start();
    }
    
    public void join() {
        osThread.join();
    }
    
    protected abstract void run();
}

// Green thread counter implementation
class GreenThreadCounter : GreenThread, ThreadSafeCounter {
    private volatile i32 count = 0;
    private GreenMutex mutex = new GreenMutex();
    
    public synchronized void increment() {
        mutex.lock();
        try {
            count++;
            yield(); // Cooperative yielding
        } finally {
            mutex.unlock();
        }
    }
    
    public synchronized void decrement() {
        mutex.lock();
        try {
            if (count > 0) {
                count--;
                yield(); // Cooperative yielding
            }
        } finally {
            mutex.unlock();
        }
    }
    
    public i32 get() {
        return count;
    }
    
    protected void run() {
        // Green thread implementation
        while (isRunning()) {
            // Work logic here
            yield();
        }
    }
}

// OS thread counter implementation
class OSThreadCounter : OSThread, ThreadSafeCounter {
    private volatile i32 count = 0;
    private Mutex mutex = new Mutex();
    
    public synchronized void increment() {
        mutex.lock();
        try {
            count++;
        } finally {
            mutex.unlock();
        }
    }
    
    public synchronized void decrement() {
        mutex.lock();
        try {
            if (count > 0) {
                count--;
            }
        } finally {
            mutex.unlock();
        }
    }
    
    public i32 get() {
        return count;
    }
    
    protected void run() {
        // OS thread implementation
        while (isRunning()) {
            // Work logic here
            Thread.sleep(10); // OS-level sleep
        }
    }
}
```

### 3. Channels (Integrated with Async/Await)

Channels provide type-safe communication between concurrent contexts and integrate seamlessly with async/await:

```blueprint
blueprint Channel<T> {
    public async sendAsync(item) {
        input: item: T;
        output: Future<void>;
        requires: !this.isClosed();
        ensures: this.itemSent(item);
    }
    
    public async receiveAsync() {
        output: Future<T?>
        ensures: (receiveAsync != null) ==> itemAvailable();
        ensures: (receiveAsync == null) ==> (isClosed() || isEmpty());
    }
    
    public send(item) {
        input: item: T;
        output: bool
        requires: !isClosed();
        ensures: send ==> itemSent(item);
    }
    
    public receive() {
        output: T?
        ensures: (receive != null) ==> itemAvailable();
        ensures: (receive == null) ==> (this.isClosed() || this.isEmpty());
    }
    
    public close() {
        output: void;
        ensures: this.isClosed();
    }
    
    public isClosed() {
        output: bool;
    }
    
    public isEmpty() {
        output: bool
    }
}

// Buffered channel implementation
class BufferedChannel<T> : Channel<T> {
    private T[] buffer;
    private i32 capacity;
    private i32 size = 0;
    private i32 head = 0;
    private i32 tail = 0;
    private bool closed = false;
    private Mutex mutex = new Mutex();
    private Condition notEmpty = new Condition();
    private Condition notFull = new Condition();
    
    public BufferedChannel(i32 capacity) {
        this.capacity = capacity;
        this.buffer = new T[capacity];
    }
    
    public async Future<void> sendAsync(T item) {
        while (true) {
            mutex.lock();
            try {
                if (closed) {
                    throw new ChannelClosedException();
                }
                
                if (size < capacity) {
                    buffer[tail] = item;
                    tail = (tail + 1) % capacity;
                    size++;
                    notEmpty.signalAll();
                    return;
                }
            } finally {
                mutex.unlock();
            }
            
            // Channel is full, yield and try again
            await Future.yield();
        }
    }
    
    public async Future<T?> receiveAsync() {
        while (true) {
            mutex.lock();
            try {
                if (size > 0) {
                    T item = buffer[head];
                    buffer[head] = null;
                    head = (head + 1) % capacity;
                    size--;
                    notFull.signalAll();
                    return item;
                }
                
                if (closed) {
                    return null;
                }
            } finally {
                mutex.unlock();
            }
            
            // Channel is empty, yield and try again
            await Future.yield();
        }
    }
    
    public bool send(T item) {
        mutex.lock();
        try {
            if (closed || size >= capacity) {
                return false;
            }
            
            buffer[tail] = item;
            tail = (tail + 1) % capacity;
            size++;
            notEmpty.signalAll();
            return true;
        } finally {
            mutex.unlock();
        }
    }
    
    public T? receive() {
        mutex.lock();
        try {
            if (size == 0) {
                return null;
            }
            
            T item = buffer[head];
            buffer[head] = null;
            head = (head + 1) % capacity;
            size--;
            notFull.signalAll();
            return item;
        } finally {
            mutex.unlock();
        }
    }
    
    public void close() {
        mutex.lock();
        try {
            closed = true;
            notEmpty.signalAll();
            notFull.signalAll();
        } finally {
            mutex.unlock();
        }
    }
    
    public bool isClosed() {
        return closed;
    }
    
    public bool isEmpty() {
        mutex.lock();
        try {
            return size == 0;
        } finally {
            mutex.unlock();
        }
    }
}
```

## Thread Safety and Synchronized Methods

BluePrint supports synchronized methods at both the blueprint contract level and implementation level for clarity and safety:

### Synchronized Blueprint Contracts

```blueprint
blueprint BankAccount {
    invariant: balance >= 0;
    invariant: isThreadSafe();
    
    public synchronized deposit(amount) {
        input: amount: fractional;
        output: void;
        requires: amount > 0;
        ensures: balance == old(balance) + amount;
        ensures: this.threadSafeAccess();
    }
    
    public synchronized withdraw(amount) {
        input: amount: fractional;
        output: bool;
        requires: amount > 0;
        ensures: withdraw ==> (balance == old(balance) - amount);
        ensures: !withdraw ==> (balance == old(balance));
        ensures: this.threadSafeAccess();
    }
    
    public getBalance() {
        output: fractional
        ensures: getBalance >= 0;
        // Note: not synchronized - snapshot read
    }
}

// Mixed synchronization in blueprints
blueprint ConcurrentCache<K, V> {
    invariant: isThreadSafe();
    
    public synchronized put(key, value) {
        input: key: K, value: V;
        output: V?;
        requires: key != null;
        ensures: this.get(key) == value;
        ensures: this.threadSafeWrite();
    }
    
    public get(key) {
        input: key: K;
        output: V?
        requires: key != null;
        // Optimistic read - may be lock-free
    }
    
    public synchronized clear() {
        output: void;
        ensures: this.isEmpty();
        ensures: this.threadSafeWrite();
    }
}
```

### Synchronized Method Implementations

```blueprint
class ThreadSafeBankAccount : BankAccount {
    private fractional balance = 0.0;
    private Mutex accountMutex = new Mutex();
    
    public synchronized void deposit(fractional amount) {
        accountMutex.lock();
        try {
            balance += amount;
            notifyBalanceChange();
        } finally {
            accountMutex.unlock();
        }
    }
    
    public synchronized bool withdraw(fractional amount) {
        accountMutex.lock();
        try {
            if (balance >= amount) {
                balance -= amount;
                notifyBalanceChange();
                return true;
            }
            return false;
        } finally {
            accountMutex.unlock();
        }
    }
    
    public fractional getBalance() {
        // Atomic read of volatile field - no lock needed
        return balance;
    }
    
    private void notifyBalanceChange() {
        // Internal method called within synchronized context
        System.out.println("Balance changed to: " + balance);
    }
}

// Blueprint contracts don't dictate implementation details
// Lock-free implementation can still satisfy synchronized contracts
class LockFreeConcurrentCache<K, V> : ConcurrentCache<K, V> {
    private AtomicReference<Node<K, V>[]> table;
    private AtomicInteger size = new AtomicInteger(0);
    
    public synchronized V? put(K key, V value) {
        // Blueprint says synchronized, but implementation can be lock-free
        // Blueprint only cares about thread-safety guarantees, not how they're achieved
        return putLockFree(key, value);
    }
    
    public V? get(K key) {
        return getLockFree(key);
    }
    
    public synchronized void clear() {
        // Atomic operation satisfies synchronization contract
        table.set(new Node[16]);
        size.set(0);
    }
    
    private V? putLockFree(K key, V value) {
        // Lock-free implementation details...
        return null; // Placeholder
    }
    
    private V? getLockFree(K key) {
        // Lock-free implementation details...
        return null; // Placeholder
    }
}
```

## Complete Example: Async/Await with Channels (Recommended Pattern)

Here's a comprehensive example showing the recommended async/await + channels approach:

```blueprint
blueprint WebServerHandler {
    public async handleRequest(request, responseChannel) {
        input: 
            request: HttpRequest,
            responseChannel: Channel<HttpResponse>;
        output: Future<void>
        requires: request != null && responseChannel != null;
        ensures: responseChannel.itemsProcessed();
    }
    
    public async processRequestBatch(requests) {
        input: requests: HttpRequest[];
        output: Future<HttpResponse[]>
        requires: requests != null;
        ensures: processRequestBatch.length == requests.length;
    }
}

class AsyncWebServer : WebServerHandler {
    private Channel<HttpRequest> requestQueue;
    private Channel<HttpResponse> responseQueue;
    private GreenThreadExecutor executor;
    
    public AsyncWebServer() {
        this.requestQueue = new BufferedChannel<HttpRequest>(1000);
        this.responseQueue = new BufferedChannel<HttpResponse>(1000);
        this.executor = new GreenThreadExecutor(numWorkers: 100);
    }
    
    public async Future<void> handleRequest(HttpRequest request, 
                                           Channel<HttpResponse> responseChannel) {
        try {
            // Process request asynchronously
            String data = await DatabaseService.queryAsync(request.getQuery());
            HttpResponse response = new HttpResponse();
            response.setData(data);
            response.setStatus(200);
            
            // Send response through channel
            await responseChannel.sendAsync(response);
            
        } catch (DatabaseException e) {
            HttpResponse errorResponse = new HttpResponse();
            errorResponse.setStatus(500);
            errorResponse.setError(e.getMessage());
            await responseChannel.sendAsync(errorResponse);
        }
    }
    
    public async Future<HttpResponse[]> processRequestBatch(HttpRequest[] requests) {
        Channel<HttpResponse> responses = new BufferedChannel<HttpResponse>(requests.length);
        
        // Start all request handlers concurrently
        Future<void>[] handlers = new Future[requests.length];
        for (i32 i = 0; i < requests.length; i++) {
            handlers[i] = handleRequest(requests[i], responses);
        }
        
        // Collect all responses
        HttpResponse[] results = new HttpResponse[requests.length];
        for (i32 i = 0; i < requests.length; i++) {
            results[i] = await responses.receiveAsync();
        }
        
        // Wait for all handlers to complete
        for (Future<void> handler : handlers) {
            await handler;
        }
        
        return results;
    }
    
    // Main server loop using async/await + channels
    public async void startServer() {
        while (isRunning()) {
            // Accept incoming requests
            HttpRequest? request = await NetworkService.acceptRequestAsync();
            if (request != null) {
                // Process asynchronously without blocking
                Future<void> handler = handleRequest(request, responseQueue);
                // Don't await - let it run concurrently
            }
            
            // Send any pending responses
            HttpResponse? response = responseQueue.receive(); // Non-blocking
            if (response != null) {
                await NetworkService.sendResponseAsync(response);
            }
        }
    }
}
```

```blueprint
blueprint ThreadSafeCollection<T> {
    invariant: isThreadSafe();
    invariant: size() >= 0;
    
    public synchronized add(item) {
        input: item: T;
        output: void;
        ensures: this.contains(item);
        ensures: this.size() == old(this.size()) + 1;
    }
    
    public synchronized remove(item) {
        input: item: T;
        output: bool;
        ensures: (remove == true) ==> (!this.contains(item));
        ensures: (remove == true) ==> (this.size() == old(this.size()) - 1);
        ensures: (remove == false) ==> (this.size() == old(this.size()));
    }
    
    public get(index) {
        input: index: u32;
        output: T;
        requires: index < this.size();
        // Note: not synchronized - may be inconsistent in multi-threaded context
    }
}
```

### Lock-Free Operations

```blueprint
blueprint AtomicOperations<T> {
    public compareAndSwap(expected, newValue) {
        input:
            expected: T,
            newValue: T;
        output: bool;
        ensures: (compareAndSwap == true) ==> (this.get() == newValue);
        ensures: (compareAndSwap == false) ==> (this.get() != expected);
    }
    
    public getAndSet(newValue) {
        input: newValue: T;
        output: T;
        ensures: this.get() == newValue;
    }
}

class AtomicInteger : AtomicOperations<i32> {
    private volatile i32 value;
    
    public bool compareAndSwap(i32 expected, i32 newValue) {
        return Atomic.compareAndSwapInt32(&value, expected, newValue);
    }
    
    public i32 getAndSet(i32 newValue) {
        return Atomic.exchangeInt32(&value, newValue);
    }
    
    public i32 get() {
        return value;
    }
}
```

## Async/Await with Error Handling

```blueprint
blueprint AsyncDatabase {
    public async query(sql) {
        input: sql: str;
        output: Future<ResultSet>
        throws: SQLException, TimeoutException;
        requires: sql != null && !sql.isEmpty();
        ensures: query != null;
    }
}

class DatabaseConnection : AsyncDatabase {
    public async Future<ResultSet> query(str sql) {
        try {
            Connection conn = await connectionPool.getConnection();
            try {
                PreparedStatement stmt = conn.prepareStatement(sql);
                return await stmt.executeQuery();
            } finally {
                connectionPool.releaseConnection(conn);
            }
        } catch (TimeoutException e) {
            throw new DatabaseTimeoutException("Query timed out: " + sql, e);
        }
    }
    
    public async void processQueries() {
        str[] queries = {"SELECT * FROM users", "SELECT * FROM orders"};
        
        // Parallel execution
        Future<ResultSet>[] futures = new Future[queries.length];
        for (i32 i = 0; i < queries.length; i++) {
            futures[i] = query(queries[i]);
        }
        
        // Collect results
        for (i32 i = 0; i < futures.length; i++) {
            try {
                ResultSet results = await futures[i];
                processResults(results);
            } catch (SQLException e) {
                System.err.println("Query failed: " + queries[i] + " - " + e.getMessage());
            }
        }
    }
}
```

## Best Practices

### 1. Primary Approach: Async/Await with Channels
- **Async/Await + Channels**: Recommended for most concurrent programming (I/O-bound operations, web services, microservices)
- **Green Threads**: Use for lightweight concurrency with thousands of concurrent tasks
- **OS Threads**: Use for CPU-intensive parallel processing that benefits from true parallelism

### 2. Threading Choice Guidelines
- **Green Threads** (`GreenThread` base class): High concurrency, cooperative scheduling, lower memory overhead
- **OS Threads** (`OSThread` base class): True parallelism, preemptive scheduling, higher memory overhead

### 3. Synchronization in Blueprints
- Use `synchronized` in both blueprint contracts AND method implementations for clarity
- Specify thread-safety invariants in blueprints (`invariant: isThreadSafe()`)
- Document blocking vs non-blocking operations in contracts
- Use `volatile` for simple shared variables
- Mixed synchronization: synchronized blueprints can have lock-free implementations

### 4. Channel Best Practices
- Use buffered channels for producer-consumer scenarios
- Prefer `sendAsync()`/`receiveAsync()` with await for primary concurrency
- Use synchronous `send()`/`receive()` only for performance-critical sections
- Always close channels when done to prevent resource leaks
- Handle channel closure gracefully in receivers

### 5. Error Handling
- Always handle `InterruptedException` in blocking operations
- Use timeouts for network operations with async methods
- Provide cancellation mechanisms for long-running async tasks
- Clean up resources in `finally` blocks or try-with-resources
- Handle channel exceptions (`ChannelClosedException`) appropriately
