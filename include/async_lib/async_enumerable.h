#pragma once

#include <memory>

#include "task.h"

namespace Async {

    template <typename T>
    class AsyncEnumerable : public std::enable_shared_from_this<AsyncEnumerable<T>> {
    public:
        using Generator = std::function<std::optional<Async::Task<T>>(void)>;

        static auto create(Scheduler::IScheduler& scheduler, Generator generate_next)
            -> std::shared_ptr<AsyncEnumerable<T>> {
            return std::shared_ptr<AsyncEnumerable<T>>(
                new AsyncEnumerable<T>(scheduler, std::move(generate_next)));
        }

        auto for_each(std::function<void(T)> func) -> auto {
            auto self = this->shared_from_this();
            std::optional<Async::Task<T>> task = generate_next_();
            if (!task.has_value()) {
                return Async::Task<Async::Unit>::immediate_task(scheduler_, Async::Unit{});
            }

            // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks): False positive - memory managed by shared_ptr and task system
            return task->template bind<Async::Unit>([self, func](T value) {
                func(value);
                return self->for_each(func);
            });
        }

    private:
        AsyncEnumerable(
            Scheduler::IScheduler& scheduler,
            Generator generate_next
        ) : scheduler_(scheduler)
          , generate_next_(std::move(generate_next))
        {
        }

        Scheduler::IScheduler& scheduler_;
        Generator generate_next_;
    };


}