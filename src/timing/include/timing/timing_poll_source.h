#pragma once

#include "timing/structures/timing_wheel_hierarchical.h"
#include "concurrency/spinlock.h"
#include "interface/timing/timer_poll_source_intf.h"

namespace Timing {
    class PollSource : public IPollSource {
    public:
        PollSource();

        [[nodiscard]] auto poll_frequency() -> std::chrono::milliseconds override;
        [[nodiscard]] auto poll() -> std::vector<Scheduler::Job> override;

        auto schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void override;

    private:
        SpinLock spinlock;
        HierarchicalTimingWheel<Scheduler::Job> wheel;
    };
}