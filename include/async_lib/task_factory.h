#pragma once

#include <memory>
#include <functional>

#include "scheduler/scheduler_factory.h"
#include "async_lib/task_io_source.h"
#include "async_lib/task_timer_source.h"
#include "task_value_source.h"
#include "task.h"

namespace Async {
    namespace IO = ::IO;

    // TaskFactory exists for the sole purpose of tying together the various components of the Async
    // library to a singular scheduler instance, it is mostly a convenience class and does not need to be used
    // if not required. It should be noted however that if one is not using this class, ideally they should be threading
    // THE SAME scheduler instance through all task instances to ensure that they are all executed on the same thread pool.
    class TaskFactory {
    public:
        explicit TaskFactory(int n_workers);

        template <typename T>
        [[nodiscard]] auto value_source() -> TaskValueSource<T>;
        [[nodiscard]] auto timer_source() -> TaskTimerSource;
        [[nodiscard]] auto io_source() -> TaskIOSource;
            
        template <typename T>
        [[nodiscard]] auto create(std::function<T(void)> function) -> Task<T>;

        template <typename T>
        [[nodiscard]] auto when_any(std::vector<Task<T>> tasks) -> Task<T>;

        template <typename T>
        [[nodiscard]] auto when_all(std::vector<Task<T>> tasks) -> Task<std::vector<T>>;

    private:
        std::shared_ptr<Timing::PollSource> timing_poll_source;
        std::shared_ptr<IO::PollSource> io_poll_source;
        std::unique_ptr<Scheduler::IScheduler> scheduler;
    };
}



// Implementation
inline Async::TaskFactory::TaskFactory(int n_workers) :
    timing_poll_source(std::make_shared<Timing::PollSource>()),
    io_poll_source(std::make_shared<IO::PollSource>()),
    scheduler(Scheduler::create_scheduler(n_workers, { timing_poll_source, io_poll_source }))
{}

template <typename T>
auto inline Async::TaskFactory::value_source() -> TaskValueSource<T> {
    return TaskValueSource<T>(*scheduler);
}

auto inline Async::TaskFactory::timer_source() -> TaskTimerSource {
    return { *scheduler, *timing_poll_source };
}

auto inline Async::TaskFactory::io_source() -> TaskIOSource {
    return { *scheduler, *io_poll_source };
}


template <typename T>
auto Async::TaskFactory::create(std::function<T(void)> function) -> Task<T> {
    return { *scheduler, function };
}

template <typename T>
auto Async::TaskFactory::when_any(std::vector<Task<T>> tasks) -> Task<T> {
    return Task<T>::when_any(*scheduler, tasks); 
}

template <typename T>
auto Async::TaskFactory::when_all(std::vector<Task<T>> tasks) -> Task<std::vector<T>> {
    return Task<T>::when_all(*scheduler, tasks); 
}