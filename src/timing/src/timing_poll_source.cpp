#include <chrono>
#include <mutex>
#include <utility>
#include <vector>

#include "timing/timing_poll_source.h"
#include "timing/structures/timing_wheel_hierarchical.h"
#include "scheduler/job.h"
#include "concurrency/spinlock.h"


// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
Timing::PollSource::PollSource() :
    wheel(Timing::HierarchicalTimingWheel<Scheduler::Job>(
        /* tick_size = */ std::chrono::milliseconds(50),
        /* wheel_sizes = */ {
            std::chrono::minutes(1) / std::chrono::milliseconds(50),    // minute wheel (ticks that map to the resolution of 1 minute)
            60, // hour wheel (ticks that map to the resolution of 1 hour)
            24, // day wheel (ticks that map to the resolution of 1 day)
            10  // 10 day wheel (we support the scheduling of events max 10 days into the future)
    })) {}

auto Timing::PollSource::schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void {
    const std::lock_guard<SpinLock> lock(spinlock);
    wheel.schedule(expiry, std::move(task));
}

auto Timing::PollSource::poll_frequency() -> std::chrono::milliseconds { return std::chrono::milliseconds(5); }
auto Timing::PollSource::poll() -> std::vector<Scheduler::Job> {
    // busy wait for the lock - we busy wait as the other thread (schedule) will not maintain
    // the lock for that long so its illogical to yield our time slice
    const std::lock_guard<SpinLock> lock(spinlock);
    auto expired_jobs = wheel.advance();
    return expired_jobs;
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)