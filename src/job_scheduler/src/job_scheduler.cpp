#include <ranges>
#include <memory>
#include <functional>
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stop_token>

#include "timing/structures/timing_wheel.h"
#include "job_scheduler/job_scheduler.h"

using std::chrono_literals::operator""ms, std::chrono_literals::operator""min;
using namespace Scheduler;


JobScheduler::JobScheduler(int n_workers, PollSources poll_sources) : job_queue(1024) {
    this->worker_queues = std::vector<JobQueue>(n_workers);
    this->worker_threads = std::vector<std::jthread>(n_workers);
    this->poll_thread = std::jthread([this, poll_sources](auto stop) {
        this->begin_poll(stop, poll_sources);
    });

    for (auto i = 0; i < n_workers; i++) {
        worker_threads[i] = std::jthread([this, i](auto stop) {
            this->begin_worker(i, stop);
        });
    }
}


auto JobScheduler::queue(Context ctx, Job job_fn) -> void { queue_batch(ctx, { job_fn }); }
auto JobScheduler::queue_batch(Context ctx, std::vector<Job> jobs) -> void {
    if (ctx == Context::empty()) {
        for (auto& job : jobs) { job_queue.enqueue(std::move(job)); }
    } else {
        for (auto& job : jobs) { worker_queues[ctx.worker_id.value()].enqueue(std::move(job)); }
    }
}


// begin_poll is the main poll loop, it will keep polling the poll sources and scheduling them in the future
// each poll source has a defined poll frequency, poll sources provide poll frequencies so that they can limit the amount of contention
// over resources they may be polling over (imagine a timer poll), the function will keep polling until the stop token is triggered
// after a poll job is complete it will then schedule it n-seconds in the future based on the poll frequency
auto JobScheduler::begin_poll(std::stop_token stop_token, PollSources poll_sources) -> void {
    auto poll_scheduler = TimingWheel<PollSource>(
        /* wheel_tick_size = */ 10ms,
        /* wheel_size = */ 1min / 10ms);

    // initially... schedule all the poll sources for execution
    auto schedule_poll_source = [&](auto time, auto& source) { poll_scheduler.schedule(time, std::move(source)); };
    for (auto& source : poll_sources) { schedule_poll_source(0ms, source); }
    
    // now continuously poll the poll sources, only running them when they are scheduled in the future
    while (!stop_token.stop_requested()) {
        for (auto& ready_poll : poll_scheduler.advance()) {
            queue_batch(Context::empty(), ready_poll->poll());
            schedule_poll_source(ready_poll->poll_frequency(), ready_poll);
        }
    }
}

// next_worker_job is a helper function that will attempt to find the next job for a worker to execute 
// it will first check the worker's queue, then the global queue, and finally evict a job from another worker
// queue
auto JobScheduler::next_worker_job(int worker_id) -> std::optional<Job> {
    if (auto job = worker_queues[worker_id].dequeue(); job.has_value()) {
        return job;
    }

    // check the global queue for any jobs
    auto job_from_global_queue = job_queue.dequeue();
    if (job_from_global_queue.has_value()) {
        return job_from_global_queue;
    }

    // we must now evict a job from another worker queue, we do this by iterating from our 
    // current worker id until we wrap around, we do this "approximately" skipping if we
    // see no jobs
    for (auto i = worker_id + 1; i != worker_id; i = (i + 1) % worker_queues.size()) {
        if (worker_queues[i % worker_queues.size()].size() == 0) { continue; }
        if (auto job = worker_queues[i % worker_queues.size()].dequeue(); job.has_value()) {
            return job;
        }
    }

    return std::nullopt;
}

// begin_worker is the main scheduler loop, it will keep picking off tasks from the run queue while there is
// still work to be done
auto JobScheduler::begin_worker(int worker_id, std::stop_token stop_token) -> void {
    auto context_for_worker = Context(worker_id);

    while (!stop_token.stop_requested()) {
        auto job = next_worker_job(worker_id);

        if (!job.has_value()) {
            std::this_thread::yield();
        } else {
            job.value()(context_for_worker);
        }
    }
}