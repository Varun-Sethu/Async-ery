#pragma once

#include <memory>

#include "async_lib/task.h"
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
            TaskValueSource(Scheduler::IScheduler& scheduler);
            
            auto complete(T value) -> void;
            auto complete(Scheduler::Context ctx, T value) -> void;

            auto create() -> Async::Task<T>;
        private:
            std::shared_ptr<Cell::WriteOnceCell<T>> task_cell;
            Scheduler::IScheduler& scheduler;
    };
}


// Implementation
template <typename T>
Async::TaskValueSource<T>::TaskValueSource(Scheduler::IScheduler& scheduler) : scheduler(scheduler) {
    this->task_cell = std::make_shared<Cell::WriteOnceCell<T>>(scheduler);
}


template <typename T>
auto Async::TaskValueSource<T>::complete(T value) -> void { this->task_cell->write(value); }

template <typename T>
auto Async::TaskValueSource<T>::complete(Scheduler::Context ctx, T value) -> void { this->task_cell->write(ctx, value); }

template <typename T>
auto Async::TaskValueSource<T>::create() -> Async::Task<T> { return Async::Task<T>(this->scheduler, this->task_cell); }