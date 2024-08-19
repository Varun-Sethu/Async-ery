#pragma once

#include <memory>
#include <functional>

#include "job_scheduler/scheduler_factory.h"
#include "async_lib/task_io_source.h"
#include "async_lib/task_timer_source.h"
#include "task_value_source.h"
#include "task.h"

namespace Async {
    // TaskFactory exists for the sole purpose of tying together the various components of the Async
    // library to a singular scheduler instance, it is mostly a convenience class and does not need to be used
    // if not required. It should be noted however that if one is not using this class, ideally they should be threading
    // THE SAME scheduler instance through all task instances to ensure that they are all executed on the same thread pool.
    class TaskFactory {
        public:
            TaskFactory(int n_workers);

            template <typename T>
            auto value_source() -> TaskValueSource<T>;

            auto timer_source() -> TaskTimerSource;

            auto io_source() -> TaskIOSource;
            
            template <typename T>
            auto create(std::function<T(void)> f) -> Task<T>;

            template <typename T>
            auto when_any(std::vector<Task<T>> tasks) -> Task<T>;

            template <typename T>
            auto when_all(std::vector<Task<T>> tasks) -> Task<std::vector<T>>;
        private:
            std::shared_ptr<TimingPollSource> timing_poll_source;
            std::shared_ptr<IOPollSource> io_poll_source;
            std::unique_ptr<Scheduler::IJobScheduler> scheduler;
    };
}



// Implementation
Async::TaskFactory::TaskFactory(int n_workers) :
    timing_poll_source(std::make_shared<Async::TimingPollSource>()),
    io_poll_source(std::make_shared<Async::IOPollSource>()),
    scheduler(Scheduler::create_scheduler(n_workers, { timing_poll_source, io_poll_source }))
{}

template <typename T>
auto Async::TaskFactory::value_source() -> TaskValueSource<T> { return TaskValueSource<T>(*scheduler); }
auto Async::TaskFactory::timer_source() -> TaskTimerSource { return TaskTimerSource(*scheduler, *timing_poll_source); }
auto Async::TaskFactory::io_source() -> TaskIOSource { return TaskIOSource(*scheduler, *io_poll_source); }


template <typename T>
auto Async::TaskFactory::create(std::function<T(void)> f) -> Task<T> { return Task<T>(*scheduler, f); }

template <typename T>
auto Async::TaskFactory::when_any(std::vector<Task<T>> tasks) -> Task<T> { return Task<T>::when_any(*scheduler, tasks); }

template <typename T>
auto Async::TaskFactory::when_all(std::vector<Task<T>> tasks) -> Task<std::vector<T>> { return Task<T>::when_all(*scheduler, tasks); }