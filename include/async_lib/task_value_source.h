#pragma once

#include <memory>

#include "async_lib/task.h"
#include "async_lib/async_result.h"
#include "cell/write_once_cell.h"
#include "scheduler/scheduler_intf.h"

namespace Async {
    // A TaskValueSource is a class that allows multiple tasks to be minted that are resolved
    // when the TaskValueSource is marked as completed. The source is driven by a singular cell
    // that is fed to all tasks spawned from the source. This is largely inspired by .NET's 
    // TaskCompletionSource. It is safe to feed the same cell to multiple tasks as tasks cannot
    // mutate their underlying cell after instantiation and the only mutation source is Complete()
    template <typename T>
    class TaskValueSource {
    public:
        explicit TaskValueSource(Scheduler::IScheduler& scheduler);
            
        [[nodiscard]] auto create() -> Async::Task<T>;

        auto error(Async::Error error) -> void;
        auto error(Scheduler::Context ctx, Async::Error error) -> void;

        auto complete(T value) -> void;
        auto complete(Scheduler::Context ctx, T value) -> void;

    private:
        //  Note: it is an invariant of the Asynchronous library that the scheduler's
        //        lifetime is longer than the lifetime of any task / cell that uses it.
        //        in the application scope it has a 'static lifetime
        std::shared_ptr<Cell::WriteOnceCell<T, Async::Error>> task_cell;
        std::reference_wrapper<Scheduler::IScheduler> scheduler;
    };
}


// Implementation
template <typename T>
Async::TaskValueSource<T>::TaskValueSource(Scheduler::IScheduler& scheduler) : scheduler(scheduler) {
    this->task_cell = std::make_shared<Cell::WriteOnceCell<T, Async::Error>>(scheduler);
}

template <typename T>
auto Async::TaskValueSource<T>::error(Async::Error error) -> void {
    this->task_cell->error(error);
}

template <typename T>
auto Async::TaskValueSource<T>::error(Scheduler::Context ctx, Async::Error error) -> void {
    this->task_cell->error(ctx, error);
}

template <typename T>
auto Async::TaskValueSource<T>::complete(T value) -> void {
    this->task_cell->write(value);
}

template <typename T>
auto Async::TaskValueSource<T>::complete(Scheduler::Context ctx, T value) -> void {
    this->task_cell->write(ctx, value);
}

template <typename T>
auto Async::TaskValueSource<T>::create() -> Async::Task<T> {
    return { this->scheduler.get(), this->task_cell };
}