#pragma once

#include <chrono>

#include "scheduler/poll_source.h"
#include "scheduler/job.h"

namespace Timing {
    class IPollSource : public Scheduler::IPollSource {
    public:
        virtual auto schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void = 0;
    };
}
