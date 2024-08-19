#pragma once

#include <memory>
#include <thread>
#include <optional>

#include "job_scheduler/job_queue.h"
#include "job_scheduler/job_scheduler_intf.h"
#include "job_scheduler/scheduling_context.h"
#include "job_scheduler/poll_source.h"

namespace Scheduler {
    class JobScheduler : public IJobScheduler {
    public:
        using PollSource = std::shared_ptr<IPollSource>;
        using PollSources = std::vector<PollSource>;


        JobScheduler(int n_workers, PollSources poll_sources);            
        auto queue(Context ctx, Job job_fn) -> void override;

    private:
        // begin_poll will initiate the polling of all poll sources, this is done one a separate
        // thread that's distinct from the worker thread. Every poll source is every n seconds, where n
        // is specified by the source.
        auto begin_poll(std::stop_token stop_token, PollSources poll_sources) -> void;

        [[nodiscard]] auto next_worker_job(int worker_id) -> std::optional<Job>;      
        auto queue_batch(Context ctx, std::vector<Job> jobs) -> void;
        auto begin_worker(int worker_id, std::stop_token stop_token) -> void;

        // TODO: make worker its own class

        JobQueue job_queue;
        std::vector<JobQueue> worker_queues;
        std::vector<std::jthread> worker_threads;
        std::jthread poll_thread;
    };
}

