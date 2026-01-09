#pragma once

#include "task.h"

namespace Async {

    template <typename T>
    class AsyncEnumerable {
    public:
        using Generator = std::function<std::optional<Async::Task<T>>(void)>;

        // AsyncEnumerable is a class that represents an asynchronous enumerable sequence
        // it is used to represent a sequence of values that can be awaited
        // it is a wrapper around a Task that returns a value of type T
        AsyncEnumerable(
            Scheduler::IScheduler& scheduler,
            Generator generate_next
        ) : scheduler(scheduler)
          , generate_next(std::move(generate_next))
        {
        }

        auto for_each(std::function<void(T)> func) -> auto {
            std::optional<Async::Task<T>> task = generate_next();
            if (!task.has_value()) {
                return Async::Task<Async::Unit>::immediate_task(scheduler, Async::Unit{});
            }

            return task->template bind<Async::Unit>([this, func](T value) {
                func(value);
                return for_each(func);
            });
        }

    private:
        Scheduler::IScheduler& scheduler;
        Generator generate_next;
    };


}