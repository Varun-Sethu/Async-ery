#pragma once

#include <optional>

#include "scheduler/job_queue.h"
#include "scheduler/job.h"
#include "scheduler/worker.h"
#include "scheduler/scheduling_context.h"

namespace Scheduler {
    class WorkerPool {
    public:
        explicit WorkerPool(unsigned int n_workers);

        auto queue(Context ctx, Job job) -> void;
        auto queue(Context ctx, std::vector<Job> jobs) -> void;

    private:
        // find_new_work attempts to find a new job to work on, if no job is found it returns std::nullopt
        // this method is specifically used by Workers when they wish to find new work, it randomly evicts jobs
        // from an arbitrary worker's queue and returns it to the caller, however it first checks that nothing is
        // in the global queue before attempting to steal work from another worker
        [[nodiscard]] auto find_new_work() -> std::optional<Job>;

        JobQueue global_queue;
        std::vector<JobWorker> workers;
    };
}
