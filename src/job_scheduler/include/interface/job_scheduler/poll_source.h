#pragma once

#include <vector>
#include <chrono>

#include "job_scheduler/job.h"

// IPollSource is a simple interface that allows for the implementation of a poll source
// poll sources are objects that must be checked periodically for new work and to drive the completion
// of any asynchronous tasks, the poll operation is meant to return a new vector scheduler jobs to run
// note that scheduler jobs are distinct from async jobs, scheduler jobs take a scheduling context
// used to run continuations on the correct worker thread
namespace Scheduler {
    class IPollSource {
    public:
        virtual ~IPollSource() = default;

        [[nodiscard]] virtual auto poll_frequency() -> std::chrono::milliseconds = 0;
        [[nodiscard]] virtual auto poll() -> std::vector<Job> = 0;
    };
}