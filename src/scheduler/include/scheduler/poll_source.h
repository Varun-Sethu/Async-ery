#pragma once

#include <vector>
#include <chrono>

#include "types.h"

namespace Async {
    // IPollSource is a simple interface that allows for the implementation of a poll source
    // poll sources are objects that must be checked periodically for new work and to drive the completion
    // of any asynchronous tasks, the poll operation is meant to return a new vector scheduler jobs to run
    // note that scheduler jobs are distinct from async jobs, scheduler jobs take a scheduling context
    // used to run continuations on the correct worker thread
    class IPollSource {
        public:
            virtual auto poll() -> std::vector<SchedulerJob> = 0;
            virtual auto poll_frequency() -> std::chrono::milliseconds = 0;
    };   
}