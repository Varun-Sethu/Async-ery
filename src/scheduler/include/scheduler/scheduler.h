#pragma once

#include <memory>
#include <thread>
#include <optional>

#include "scheduler/scheduler_intf.h"
#include "scheduler/poll_source.h"
#include "scheduler/worker_pool.h"
#include "scheduler/scheduling_context.h"

namespace Scheduler {
    using PollSource = std::shared_ptr<IPollSource>;
    using PollSources = std::vector<PollSource>;
    
    // The core Scheduler is a work stealing scheduler based on a sequence of worker pools. Worker's are responsible for
    // executing work and all continuations are scheduled on the queue for that worker that just executed the job.
    // Workers that run out of work can evict work from other workers. Alongside this the scheduler also has a poll thread
    // this poll thread is responsible for polling a set of poll sources and queueing the jobs returned by the poll sources.
    // Poll sources can do a variety of things... poll timers, poll asynchronous IO, etc. Essentially anything that doesn't directly
    // fit into the continuation model for Async can be implemented via a poll source.
    class Scheduler : public IScheduler {
    public:
        Scheduler(unsigned int n_workers, const PollSources& poll_sources);            

        // queue will queue a job to be executed by the scheduler
        auto queue(Context ctx, std::vector<Job> jobs) -> void;
        auto queue(Context ctx, Job job_fn) -> void override;

    private:
        auto begin_poll(const std::stop_token& stop_token, PollSources poll_sources) -> void;

        WorkerPool worker_pool;
        std::jthread poll_thread;
    };
}

