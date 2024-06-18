#pragma once

#include "poll_source.h"
#include "util/timing_wheel_hierarchical.h"
#include "util/spinlock.h"

namespace Async {
    class TimingPollSource : public Async::IPollSource {
        public:
            TimingPollSource();

            auto schedule(std::chrono::milliseconds expiry, Async::job task) -> void;
            auto poll_frequency() -> std::chrono::milliseconds;
            auto poll() -> std::vector<Async::job> override;

        private:
            SpinLock mutex;
            HierarchicalTimingWheel<Async::job> wheel;
    };
}