#pragma once

#include <memory>
#include <deque>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <stop_token>

#include "types.h"
#include "polling/poll_source.h"

namespace Async {
    // Scheduler is the underlying Async scheduler class, it is responsible for executing any tasks
    // that are delegated onto it as well as periodically checking any "poll" sources 
    class Scheduler {
        using PollSource = std::shared_ptr<IPollSource>;
        
        public:
            Scheduler(int n_workers, std::vector<PollSource> poll_sources);
            auto queue(Async::job job_fn) -> void;

        private:
            auto queue_batch(std::vector<Async::job> jobs) -> void;
            auto begin_poll(std::stop_token stop_token, std::vector<PollSource> poll_sources) -> void;            
            auto begin_worker(std::stop_token stop_token) -> void;

            std::mutex queue_mutex;
            std::deque<Async::job> job_queue = std::deque<Async::job>();
            std::condition_variable_any queue_has_data;

            std::vector<std::jthread> worker_threads;
            std::jthread poll_thread;
    };
}