#include <iostream>
#include <chrono>
#include <mutex>

#include "task_value_source.h"
#include "scheduler/scheduler_intf.h"
#include "types.h"
#include "timing/timing_poll_source.h"


namespace Async {
    class TaskTimerSource {
        public:
            TaskTimerSource(Scheduler::IScheduler& scheduler, TimingPollSource& timing_poll_source) : 
                scheduler(scheduler),
                timing_poll_source(timing_poll_source) {}


            // create creates a new task that is resolved after the specified duration
            auto after(std::chrono::milliseconds duration) -> Async::Task<Unit>;

        private:
            Scheduler::IScheduler& scheduler;
            Async::TimingPollSource& timing_poll_source;
    };
}