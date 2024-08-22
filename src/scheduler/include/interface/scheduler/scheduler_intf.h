#pragma once

#include <concepts>

#include "scheduler/job.h"
#include "scheduler/scheduling_context.h"

// Conforming to this concept implies that some type T is indeed a scheduler and can be used as such
// for scheduling jobs
namespace Scheduler {
    class IScheduler {
    public:
        virtual ~IScheduler() = default;
        auto virtual queue(Context ctx, Job job_fn) -> void = 0;


        IScheduler() = default;
        IScheduler(IScheduler&&) = delete;
        IScheduler(const IScheduler&) = delete;

        auto operator=(const IScheduler&) -> IScheduler& = delete;
        auto operator=(IScheduler&&) -> IScheduler& = delete;
    };
}