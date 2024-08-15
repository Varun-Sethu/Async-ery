# Async-ery
Just some shoddy Async library for C++, features timed task sources and a relatively simple API.

# Usage
The primary type for this library is the Task<T> type. Tasks can be created from the TaskFactory, they take a function to run on some background thread and resolve when that function finishes termination. Much
of the design for this library is inspired by Jane Street's Async library for Ocaml as well as the .NET Task libraries, except in a multi-threaded context instead of a single threaded once.

## Examples
Below are some examples of how to use some of the features of the library.
### Simple tasks
```cpp
auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
auto task = task_factory.create<int>([]() {
    auto fact = [](int n) {
        if (n == 0 || n == 1) { return 1; }
        return n * fact(n - 1);
    };

    return fact(30);
});

// go do some things while task is computing
// ...
// ...    
// when you want to, block on it
std::cout << task.block();
```

### Externally Resolved tasks
Not all async computations fit into the delegate model, and instead are resolved by some external event. This can be done via task value sources, they allow us to create tasks that are resolved when a value is set by the TaskValueSource.
```cpp
auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
auto value_source = task_factory.value_source<int>();

auto task = value_source.create().map<int>([](int x) { return x + 3; });
// go do some things
// ...
// ...
// resolve the task by filling the value source, then print the value
value_source.complete(100);
std::cout << task.block();
```

### Timer Resolved tasks
Some tasks are resolved at some future point in time, these can be created via TaskTimerSources. Below is an example, note this example also showcases the "bind" method for tasks.
```cpp
auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
auto timer_source = task_factory.timer_source();
auto value_source = task_factory.value_source<int>();

// delays some integer task by (amount) milliseconds
auto delay_by = [](std::chrono::milliseconds amount) {
    return [&timer_source, amount](int value) -> Async::Task<int> {
        auto delayed_task = timer_source.after(amount) // returns a task that resolves after (amount)
        return delayed_task.map<int>([value](auto _) -> int { return value; });
    }
}

// usage is rather simple :D
auto task = value_source.create()
                .bind<int>(delay_by(200ms))
                .map<int>([](int x) { return 2 * x });
// go do some things
// ...
// ...
// resolve the task by filling the value source, then print the value
// the timer task is resolved automatically by the scheduler, under the hood
// the scheduler is fed a timing wheel that will incrementally tick off timers
value_source.complete(100);
std::cout << task.block();
```

### IO Tasks
A very "obvious" kind of async computation is that of File IO. Currently the library only supports async read operations, an example of using the library to read files asynchronously is provided below.
```cpp
auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
auto io_source = task_factory.io_source();
    
auto fp     = std::unique_ptr<FILE, decltype(&fclose)>(fopen("tests/io.txt", "r"), &fclose);
auto io_req = Async::IOReadRequest(1024, 0);
io_source.read(fp.get(), io_req)
         .map<Async::Unit>(read_file_body)
         .block();

std::cout << "=== Completed ===" << std::endl;
```


### Combinators
Alongside these simple basics, tasks can also be combined using the `when_any` and `when_all` combinators. Using them is also rather simple, a (truncated) example is found below.
```cpp
// resolves min(300ms, 500ms) = 300ms after task source is filled
auto when_any_task = task_factory.when_any<int>({
    task_source.create().bind<int>(delay_by(300ms)).map<int>([](int x) { return x + 5; }),
    task_source.create().bind<int>(delay_by(500ms)).map<int>([](int x) { return 4 * x + 5; }),
});

// resolves max(300ms, 500ms, 400ms) = 500ms after the task source is filled
auto when_all_task = task_factory.when_all<int>({
    task_source.create().bind<int>(delay_by(300ms)).map<int>([](int x) { return x + 5; }),
    task_source.create().bind<int>(delay_by(500ms)).map<int>([](int x) { return 4 * x + 5; }),
    task_source.create().bind<int>(delay_by(400ms)).map<int>([](int x) { return 3 * x + 5; }),
});

// when any returns an int
std::cout << when_any.block() << '\n';

// when all returns a vector<int>
for (auto x: when_all_task.block()) { std::cout << x << '\n'; }

```

## TODO
 - Clean up namespace pollution and fix module boundaries
    - Expose API target for each library within src/
 - Error task results
 - Write IO tasks