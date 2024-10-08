#include <chrono>
#include <thread>
#include <vector>
#include <stop_token>
#include <utility>

#include "timing/structures/timing_wheel.h"
#include "scheduler/scheduler.h"
#include "scheduler/scheduling_context.h"
#include "scheduler/job.h"

Scheduler::Scheduler::Scheduler(unsigned int n_workers, const PollSources& poll_sources) : worker_pool(n_workers) {
    this->poll_thread = std::jthread([this, poll_sources](auto stop) {
        this->begin_poll(stop, poll_sources);
    });
}

auto Scheduler::Scheduler::queue(Context ctx, Job job_fn) -> void { queue(ctx, std::vector<Job> { job_fn }); }
auto Scheduler::Scheduler::queue(Context ctx, std::vector<Job> jobs) -> void {
    this->worker_pool.queue(ctx, std::move(jobs));
}

// begin_poll is the main poll loop, it will keep polling the poll sources and scheduling them in the future
// each poll source has a defined poll frequency, poll sources provide poll frequencies so that they can limit the amount of contention
// over resources they may be polling over (imagine a timer poll), the function will keep polling until the stop token is triggered
// after a poll job is complete it will then schedule it n-seconds in the future based on the poll frequency
auto Scheduler::Scheduler::begin_poll(const std::stop_token& stop_token, PollSources poll_sources) -> void {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    auto poll_scheduler = Timing::TimingWheel<PollSource>(
        /* wheel_tick_size = */ std::chrono::milliseconds(10),
        /* num_ticks = */ std::chrono::minutes(1) / std::chrono::milliseconds(10));
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // initially... schedule all the poll sources for execution
    auto schedule_poll_source = [&](auto time, auto& source) { poll_scheduler.schedule(time, std::move(source)); };
    for (auto& source : poll_sources) { schedule_poll_source(std::chrono::milliseconds(0), source); }
    
    // now continuously poll the poll sources, only running them when they are scheduled in the future
    while (!stop_token.stop_requested()) {
        for (auto& ready_poll : poll_scheduler.advance()) {
            queue(Context::empty(), ready_poll->poll());
            schedule_poll_source(ready_poll->poll_frequency(), ready_poll);
        }
    }
}