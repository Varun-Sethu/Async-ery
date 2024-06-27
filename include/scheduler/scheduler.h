#pragma once

#include <memory>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <optional>

#include "types.h"
#include "scheduler/types.h"
#include "polling/poll_source.h"

namespace Async {
    // Scheduler is the underlying Async scheduler class, it is responsible for executing any tasks
    // that are delegated onto it as well as periodically checking any "poll" sources 
    class Scheduler {
        public:
            using PollSource = std::shared_ptr<IPollSource>;
            using Job = std::function<void(SchedulingContext)>;
        
        public:
            Scheduler(int n_workers, std::vector<PollSource> poll_sources);
            
            auto queue(SchedulingContext ctx, SchedulerJob job_fn) -> void;
            auto empty_context() -> SchedulingContext;

        private:
            auto queue_batch(SchedulingContext ctx, std::vector<SchedulerJob> jobs) -> void;

            auto begin_poll(std::stop_token stop_token, std::vector<PollSource> poll_sources) -> void;            
            auto begin_worker(int worker_id, std::stop_token stop_token) -> void;

            std::mutex queue_mutex;
            std::deque<SchedulerJob> job_queue = std::deque<SchedulerJob>();
            std::condition_variable_any queue_has_data;

            std::vector<std::jthread> worker_threads;
            std::jthread poll_thread;
    };
}