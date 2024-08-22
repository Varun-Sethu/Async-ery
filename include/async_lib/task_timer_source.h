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
            // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
            //  Note: it is an invariant of the Asynchronous library that the scheduler's
            //        lifetime is longer than the lifetime of any task / cell that uses it.
            //        in the application scope it has a 'static lifetime
            Scheduler::IScheduler& scheduler;
            // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

            // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
            //  Note: it is an invariant of the Asynchronous library that the timing_poll_source's
            //        lifetime is longer than TaskTimerSource's lifetime
            Async::TimingPollSource& timing_poll_source;
            // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
}