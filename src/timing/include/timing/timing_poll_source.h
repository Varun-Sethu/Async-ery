#pragma once

#include "timing/structures/timing_wheel_hierarchical.h"
#include "concurrency/spinlock.h"
#include "scheduler/poll_source.h"
#include "scheduler/job.h"

namespace Timing {
    class PollSource : public Scheduler::IPollSource {
        public:
            PollSource();

            auto schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void;
            auto poll_frequency() -> std::chrono::milliseconds override;
            auto poll() -> std::vector<Scheduler::Job> override;

        private:
            SpinLock spinlock;
            HierarchicalTimingWheel<Scheduler::Job> wheel;
    };
}