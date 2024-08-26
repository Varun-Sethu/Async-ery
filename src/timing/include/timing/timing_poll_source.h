#pragma once

#include "timing/structures/timing_wheel_hierarchical.h"
#include "concurrency/spinlock.h"
#include "scheduler/poll_source.h"
#include "scheduler/job.h"

namespace Timing {
    class PollSource : public Scheduler::IPollSource {
    public:
        PollSource();

        [[nodiscard]] auto poll_frequency() -> std::chrono::milliseconds override;
        [[nodiscard]] auto poll() -> std::vector<Scheduler::Job> override;

        auto schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void;

    private:
        SpinLock spinlock;
        HierarchicalTimingWheel<Scheduler::Job> wheel;
    };
}