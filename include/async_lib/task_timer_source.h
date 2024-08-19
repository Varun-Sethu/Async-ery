#include <iostream>
#include <chrono>
#include <mutex>

#include "task_value_source.h"
#include "job_scheduler/job_scheduler_intf.h"
#include "types.h"
#include "timing/timing_poll_source.h"


namespace Async {
    class TaskTimerSource {
        public:
            TaskTimerSource(Scheduler::IJobScheduler& scheduler, TimingPollSource& timing_poll_source) : 
                scheduler(scheduler),
                timing_poll_source(timing_poll_source) {}


            // create creates a new task that is resolved after the specified duration
            auto after(std::chrono::milliseconds duration) -> Async::Task<Unit>;

        private:
            Scheduler::IJobScheduler& scheduler;
            Async::TimingPollSource& timing_poll_source;
    };
}