#include <ranges>
#include <memory>
#include <functional>
#include <chrono>

#include "scheduler.h"
#include "util/timing_wheel.h"

using std::chrono_literals::operator""ms, std::chrono_literals::operator""min;

Async::Scheduler::Scheduler(int n_workers, std::vector<Async::Scheduler::PollSource> poll_sources) {
    auto workers = std::views::iota(0, n_workers)
            | std::views::transform([this](auto _) { return [this](auto stop) { this->begin_worker(stop); }; })
            | std::views::transform([](auto worker) { return std::jthread(worker); });

    this->worker_threads = std::vector<std::jthread>(workers.begin(), workers.end());
    this->poll_thread = std::jthread([this, poll_sources](auto stop) { this->begin_poll(stop, poll_sources); });
}


auto Async::Scheduler::queue(job job_fn) -> void { queue_batch({ job_fn }); }
auto Async::Scheduler::queue_batch(std::vector<job> jobs) -> void {
    {
        auto lock = std::unique_lock<std::mutex>(this->queue_mutex);
        this->job_queue.insert(this->job_queue.end(), jobs.begin(), jobs.end());
    }
    this->queue_has_data.notify_one();
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
            queue_batch(ready_poll->poll());
            schedule_poll_source(ready_poll->poll_frequency(), ready_poll);
        }
    }
}

// begin_worker is the main scheduler loop, it will keep picking off tasks from the run queue while there is
// still work to be done
auto Async::Scheduler::begin_worker(std::stop_token stop_token) -> void {
    while (!stop_token.stop_requested()) {
        auto job = ({
            auto lock = std::unique_lock<std::mutex>(this->queue_mutex);
            if (job_queue.empty()) { 
                // the condition variable may have been triggered by a stop token, in which case 
                // job_available will be false and we should terminate
                auto job_available = queue_has_data.wait(lock, stop_token, [&]() { return !job_queue.empty(); }); 
                if (!job_available) { break; }
            }

            auto job = job_queue.front();
            job_queue.pop_front();
            job;
        });

        // we cannot run the job while holding the lock as the job may try to queue more jobs
        // which would require the lock to be re-acquired - hence a deadlock D:
        job();
    }
}