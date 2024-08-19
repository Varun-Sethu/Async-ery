#pragma once

#include <concepts>

#include "job_scheduler/job.h"
#include "job_scheduler/scheduling_context.h"

// Conforming to this concept implies that some type T is indeed a scheduler and can be used as such
// for scheduling jobs
namespace Scheduler {
    class IJobScheduler {
    public:
        auto virtual queue(Context ctx, Job job_fn) -> void = 0;
    };
}