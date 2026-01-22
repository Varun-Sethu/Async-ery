#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "scheduler/scheduler_factory.h"
#include "scheduler/poll_source.h"
#include "interface/timing/timer_poll_source_intf.h"
#include "interface/io/io_poll_source_intf.h"
#include "timing/timing_poll_source.h"
#include "io/io_poll_source.h"
#include "async_lib/task_io_source.h"
#include "async_lib/task_timer_source.h"
#include "async_lib/async_enumerable.h"
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
        explicit TaskFactory(int n_workers, std::vector<std::shared_ptr<Scheduler::IPollSource>> additional_poll_sources = {});
        TaskFactory(
            std::shared_ptr<Scheduler::IScheduler> scheduler,
            std::shared_ptr<Timing::IPollSource> timer_source,
            std::shared_ptr<IO::IPollSource> io_source
        );

        template <typename T>
        [[nodiscard]] auto value_source() -> TaskValueSource<T>;
        [[nodiscard]] auto timer_source() -> TaskTimerSource;
        [[nodiscard]] auto io_source() -> TaskIOSource;
        template <typename T>
        [[nodiscard]] auto create(std::function<T(void)> function) -> Task<T>;

        template <typename T>
        [[nodiscard]] auto create_enumerable(typename AsyncEnumerable<T>::Generator generator) -> std::shared_ptr<Async::AsyncEnumerable<T>>;

        template <typename T>
        [[nodiscard]] auto when_any(std::vector<Task<T>> tasks) -> Task<T>;

        template <typename T>
        [[nodiscard]] auto when_all(std::vector<Task<T>> tasks) -> Task<std::vector<T>>;

    private:
        std::shared_ptr<Timing::IPollSource> timing_poll_source;
        std::shared_ptr<IO::IPollSource> io_poll_source;
        std::shared_ptr<Scheduler::IScheduler> scheduler;
    };
}



// Implementation
inline Async::TaskFactory::TaskFactory(int n_workers, std::vector<std::shared_ptr<Scheduler::IPollSource>> poll_sources) :
    timing_poll_source(std::make_shared<Timing::PollSource>()),
    io_poll_source(std::make_shared<::IO::PollSource>())
{
    poll_sources.insert(poll_sources.begin(), { timing_poll_source, io_poll_source });
    scheduler = Scheduler::create_scheduler(n_workers, poll_sources);
}

inline Async::TaskFactory::TaskFactory(
    std::shared_ptr<Scheduler::IScheduler> scheduler,
    std::shared_ptr<Timing::IPollSource> timer_source,
    std::shared_ptr<IO::IPollSource> io_source
) :
    timing_poll_source(std::move(timer_source)),
    io_poll_source(std::move(io_source)),
    scheduler(std::move(scheduler))
{}

template <typename T>
auto inline Async::TaskFactory::value_source() -> TaskValueSource<T> {
    return TaskValueSource<T>(*scheduler);
}

template <typename T>
auto inline Async::TaskFactory::create_enumerable(typename AsyncEnumerable<T>::Generator generator) -> std::shared_ptr<Async::AsyncEnumerable<T>> {
    return Async::AsyncEnumerable<T>::create(*scheduler, std::move(generator));
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