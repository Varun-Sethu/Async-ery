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


JobScheduler::JobScheduler(int n_workers, PollSources poll_sources) : worker_pool(n_workers) {
    this->poll_thread = std::jthread([this, poll_sources](auto stop) {
        this->begin_poll(stop, poll_sources);
    });
}


auto JobScheduler::queue(Context ctx, Job job_fn) -> void { queue(ctx, std::vector<Job> { job_fn }); }
auto JobScheduler::queue(Context ctx, std::vector<Job> jobs) -> void {
    this->worker_pool.queue(ctx, jobs);
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
            queue(Context::empty(), ready_poll->poll());
            schedule_poll_source(ready_poll->poll_frequency(), ready_poll);
        }
    }
}