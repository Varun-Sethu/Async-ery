#include <ranges>
#include <memory>
#include <functional>
#include <iostream>
#include <chrono>

#include "scheduler/scheduler.h"
#include "util/timing_wheel.h"

using std::chrono_literals::operator""ms, std::chrono_literals::operator""min;

Async::Scheduler::Scheduler(int n_workers, std::vector<Async::Scheduler::PollSource> poll_sources) : job_queue(1024) {
    auto queues = std::views::iota(0, n_workers) | std::views::transform([](auto id) { return CircularQueue(1024); });
    this->worker_queues = std::vector<CircularQueue>(
        std::move_iterator(queues.begin()),
        std::move_iterator(queues.end()));

    auto workers = std::views::iota(0, n_workers)
            | std::views::transform([this](auto id) { return [this, id](auto stop) { this->begin_worker(id, stop); }; })
            | std::views::transform([](auto worker) { return std::jthread(worker); });

    this->worker_threads = std::vector<std::jthread>(workers.begin(), workers.end());
    this->poll_thread = std::jthread([this, poll_sources](auto stop) { this->begin_poll(stop, poll_sources); });
}


auto Async::Scheduler::queue(SchedulingContext ctx, SchedulerJob job_fn) -> void { queue_batch(ctx, { job_fn }); }
auto Async::Scheduler::queue_batch(SchedulingContext ctx, std::vector<SchedulerJob> jobs) -> void {
    if (ctx == SchedulingContext::empty()) {
        for (auto& job : jobs) { job_queue.enqueue(std::move(job)); }
    } else {
        for (auto& job : jobs) { worker_queues[ctx.worker_id.value()].enqueue(std::move(job)); }
    }
}


// begin_poll is the main poll loop, it will keep polling the poll sources and scheduling them in the future
// each poll source has a defined poll frequency, poll sources provide poll frequencies so that they can limit the amount of contention
// over resources they may be polling over (imagine a timer poll), the function will keep polling until the stop token is triggered
// after a poll job is complete it will then schedule it n-seconds in the future based on the poll frequency
auto Async::Scheduler::begin_poll(std::stop_token stop_token, std::vector<Async::Scheduler::PollSource> poll_sources) -> void {
    auto poll_scheduler = TimingWheel<Async::Scheduler::PollSource>(
        /* wheel_tick_size = */ 10ms,
        /* wheel_size = */ 1min / 10ms);

    // initially... schedule all the poll sources for execution
    auto schedule_poll_source = [&](auto time, auto& source) { poll_scheduler.schedule(time, std::move(source)); };
    for (auto& source : poll_sources) { schedule_poll_source(0ms, source); }
    
    // now continuously poll the poll sources, only running them when they are scheduled in the future
    while (!stop_token.stop_requested()) {
        for (auto& ready_poll : poll_scheduler.advance()) {
            queue_batch(SchedulingContext::empty(), ready_poll->poll());
            schedule_poll_source(ready_poll->poll_frequency(), ready_poll);
        }
    }
}

// next_worker_job is a helper function that will attempt to find the next job for a worker to execute 
// it will first check the worker's queue, then the global queue, and finally evict a job from another worker
// queue
auto Async::Scheduler::next_worker_job(int worker_id) -> std::optional<SchedulerJob> {
    if (auto job = worker_queues[worker_id].dequeue(); job.has_value()) { return job; }

    // check the global queue for any jobs, we do this "naively" by first testing for a job
    // using the size (this maintains a relaxed memory order) and if one exists then acquiring it
    auto job_from_global_queue = job_queue.size() > 0 
                                    ? job_queue.dequeue() 
                                    : std::nullopt;
    
    if (job_from_global_queue.has_value()) { return job_from_global_queue; }

    // we must now evict a job from another worker queue, we do this by iterating from our 
    // current worker id until we wrap around, we do this "approximately" skipping if we
    // see no jobs
    for (auto i = worker_id + 1; i != worker_id; i = (i + 1) % worker_queues.size()) {
        if (worker_queues[i].size() == 0) { continue; }
        if (auto job = worker_queues[i].dequeue(); job.has_value()) {
            return job;
        }
    }

    return std::nullopt;
}

// begin_worker is the main scheduler loop, it will keep picking off tasks from the run queue while there is
// still work to be done
auto Async::Scheduler::begin_worker(int worker_id, std::stop_token stop_token) -> void {
    auto context_for_worker = SchedulingContext(worker_id);

    while (!stop_token.stop_requested()) {
        auto job = next_worker_job(worker_id);

        if (!job.has_value()) { std::this_thread::yield(); }
        else {
            job.value()(context_for_worker);
        }
    }
}