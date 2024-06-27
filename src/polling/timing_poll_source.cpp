#include <chrono>
#include <mutex>

#include "polling/timing_poll_source.h"

using std::chrono_literals::operator""ms, std::chrono_literals::operator""min;

Async::TimingPollSource::TimingPollSource() : 
    wheel(HierarchicalTimingWheel<Async::SchedulerJob>(
        /* wheel_tick_size = */ 50ms,
        /* level_sizes = */ {
            1min / 50ms,    // minute wheel (ticks that map to the resolution of 1 minute)
            60,             // hour wheel (ticks that map to the resolution of 1 hour)
            24,             // day wheel (ticks that map to the resolution of 1 day)
            10              // 10 day wheel (we support the scheduling of events max 10 days into the future)
    })) {}


auto Async::TimingPollSource::schedule(std::chrono::milliseconds expiry, Async::SchedulerJob task) -> void {
    std::lock_guard<SpinLock> lock(spinlock);
    wheel.schedule(expiry, std::move(task));
}

auto Async::TimingPollSource::poll_frequency() -> std::chrono::milliseconds { return 5ms; }
auto Async::TimingPollSource::poll() -> std::vector<Async::SchedulerJob> {
    // busy wait for the lock - we busy wait as the other thread (schedule) will not maintain
    // the lock for that long so its illogical to yield our time slice
    std::lock_guard<SpinLock> lock(spinlock);
    auto expired_jobs = wheel.advance();
    return expired_jobs;
}