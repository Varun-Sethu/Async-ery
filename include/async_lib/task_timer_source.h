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
        TaskTimerSource(Scheduler::IScheduler& scheduler, Timing::PollSource& timing_poll_source) : 
            scheduler(scheduler),
            timing_poll_source(timing_poll_source) {}


        // create creates a new task that is resolved after the specified duration
        auto after(std::chrono::milliseconds duration) -> Async::Task<Unit>;

    private:
        // Note:
        //      It is expected that the lifetime of the scheduler is longer than the lifetime of the TaskTimerSource
        //      the scheduler's lifetime should match the entire application lifetime.
        //
        //      It is expected that the lifetime of the timing_poll_source is longer than the lifetime of the TaskTimerSource
        //      the timing_poll_source's lifetime should match the entire application lifetime. This is trivially true as
        //      the timing_poll_source is captured via shared ownership by the scheduler, hence its lifetime is the same
        std::reference_wrapper<Scheduler::IScheduler> scheduler;
        std::reference_wrapper<Timing::PollSource> timing_poll_source;
    };
}