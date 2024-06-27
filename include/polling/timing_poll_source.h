#pragma once

#include "poll_source.h"
#include "util/timing_wheel_hierarchical.h"
#include "util/spinlock.h"
#include "scheduler/types.h"

namespace Async {
    class TimingPollSource : public Async::IPollSource {
        public:
            TimingPollSource();

            auto schedule(std::chrono::milliseconds expiry, Async::SchedulerJob task) -> void;
            auto poll_frequency() -> std::chrono::milliseconds;
            auto poll() -> std::vector<Async::SchedulerJob> override;

        private:
            SpinLock spinlock;
            HierarchicalTimingWheel<Async::SchedulerJob> wheel;
    };
}