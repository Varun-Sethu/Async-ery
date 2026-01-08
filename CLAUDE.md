# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Debug build with sanitizers
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run examples (they serve as integration tests)
./build/examples/main_example
./build/examples/error_example
./build/examples/io_example
./build/examples/concurrency
```

## Architecture Overview

**Async-ery** is a multi-threaded async library for C++20 inspired by Jane Street's Async (OCaml) and .NET's Task libraries. It provides composable, type-safe async computations across a work-stealing thread pool.

### Core Layers (Bottom-Up)

1. **Cells** (`src/cell/`): Thread-safe, write-once data containers that form the foundation
   - `WriteOnceCell<T, Err>` - immediate callback execution on value availability
   - `TrackingOnceCell<T, Err>` - delays subscription until tracked cell is known (used for `bind()`)
   - `WhenAllCell` / `WhenAnyCell` - combinators for multiple cells
   - All cells implement `ICell<T, Err>` interface with `await(callback)`, `read()`, `block()`

2. **Scheduler** (`src/scheduler/`): Work-stealing thread pool
   - `Scheduler` - manages worker pool and poll thread
   - `WorkerPool` - work-stealing across workers, global queue + per-worker queues
   - `JobWorker` - individual worker thread with local queue and steal capability
   - `JobQueue` - lock-free queue with spinlock, cache-line aligned
   - `Context` - carries worker ID for scheduling continuations on specific workers

3. **Timing** (`src/timing/`): Hierarchical timing wheel for efficient timer management
   - Scales to millions of timers with O(1) scheduling
   - Integrated via `IPollSource` polled by scheduler's poll thread

4. **IO** (`src/io/`): Async file IO via POSIX AIO
   - `AIOManager` enqueues async reads, polled for completion via `IPollSource`

5. **Public API** (`include/async_lib/`):
   - `Task<T>` - primary async computation type with `map()`, `bind()`, `block()`, `when_any()`, `when_all()`
   - `TaskFactory` - bundles scheduler with task creation methods
   - `TaskValueSource<T>` - externally-resolved tasks (like .NET's TaskCompletionSource)
   - `TaskTimerSource` - creates tasks from durations (`after()`, `periodic()`)
   - `TaskIOSource` - async file read operations
   - `Result<T>` = `std::variant<T, Error>` for error handling

### Task Composition Flow

```
WriteOnceCell (immediate work)
    ↓ map()
WriteOnceCell (transformation)
    ↓ bind()
TrackingOnceCell (dependent task)
    ↓ block()
(synchronous wait)
```

Errors propagate through chains via `Result<T, Error>` variants with `Error::Rejected` and `Error::IOError`.

### Key Design Decisions

- **Shared ownership**: Tasks share underlying cells via `shared_ptr`
- **Lifetime invariant**: Scheduler must outlive all tasks
- **Poll integration**: External events (timers, IO) integrated via `IPollSource` polled by dedicated thread
- **Cache locality**: Context-aware scheduling, cache-line aligned queues to prevent false sharing

## Code Style

- Use RAII for resources (unique_ptr for file descriptors)
- Use `auto` for local variable types
- Use `const auto` (const before type)
- Use `const auto&` where appropriate
- No single-line if statements
- Use auto in for loop variables
- Do not add comments to the code
- For complex boolean conditions, extract to a descriptive variable:
  ```cpp
  auto is_cancellable = !state_->fired && !state_->cancelled;
  if (!is_cancellable) { ... }
  ```
