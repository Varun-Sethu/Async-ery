#pragma once

#include <optional>
#include <thread>

#include "scheduler/job_queue.h"
#include "scheduler/job.h"

namespace Scheduler {
    using StealWork = std::function<std::optional<Job>()>;

    // JobWorkers are responsible for executing jobs. Whenever the worker is out of work
    // it can choose to steal a job from another worker using the handle provided by the WorkerPool
    class JobWorker {
    public:
        JobWorker(Context worker_context, StealWork steal_work);

        // start begins the individual worker on a new thread, it's worth noting that this is slightly different from the rest of the codebase
        // this is because we want to be able to create all the workers beforehand (specifically their worker queues) and then start them
        // once they've all been created. The reason we do this is to prevent a situation where a worker is started and then immediately
        // attempts to steal work from a worker that hasn't been created yet.
        [[nodiscard]] auto start() -> bool;
        [[nodiscard]] auto steal_job() -> std::optional<Job>;

        auto queue(std::vector<Job> jobs) -> void;
        auto queue(Job job) -> void;
    private:
        Context worker_context;
        StealWork steal_work;
        JobQueue job_queue;
        std::optional<std::jthread> worker_thread;
    };
}