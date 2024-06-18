#include <chrono>

#include "polling/timing_poll_source.h"

using std::chrono_literals::operator""ms, std::chrono_literals::operator""min;

Async::TimingPollSource::TimingPollSource() : 
    wheel(HierarchicalTimingWheel<Async::job>(
        /* wheel_tick_size = */ 50ms,
        /* level_sizes = */ {
            1min / 50ms,    // minute wheel (ticks that map to the resolution of 1 minute)
            60,             // hour wheel (ticks that map to the resolution of 1 hour)
            24,             // day wheel (ticks that map to the resolution of 1 day)
            10              // 10 day wheel (we support the scheduling of events max 10 days into the future)
    })) {}


auto Async::TimingPollSource::schedule(std::chrono::milliseconds expiry, Async::job task) -> void {
    auto lock = mutex.lock();
    wheel.schedule(expiry, std::move(task));
}

auto Async::TimingPollSource::poll_frequency() -> std::chrono::milliseconds { return 5ms; }
auto Async::TimingPollSource::poll() -> std::vector<Async::job> {
    // busy wait for the lock - we busy wait as the other thread (schedule) will not maintain
    // the lock for that long so its illogical to yield our time slice
    auto lock = mutex.lock();
    auto expired_jobs = wheel.advance();
    return expired_jobs;
}