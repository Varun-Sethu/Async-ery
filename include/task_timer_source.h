#pragma once

#include <iostream>
#include <chrono>
#include <mutex>

#include "task_value_source.h"
#include "scheduler.h"
#include "types.h"
#include "polling/timing_poll_source.h"


namespace Async {
    class TaskTimerSource {
        public:
            TaskTimerSource(Async::Scheduler& scheduler, TimingPollSource& timing_poll_source) : 
                scheduler(scheduler),
                timing_poll_source(timing_poll_source) {}


            // create creates a new task that is resolved after the specified duration
            auto after(std::chrono::milliseconds duration) -> Async::Task<unit> {
                auto value_source = std::make_shared<Async::TaskValueSource<unit>>(scheduler);
                // the value source triggers after the expiry, this is achieved by
                // scheduling a task to complete the value source after the expiry
                timing_poll_source.schedule(duration, [value_source] () { value_source->complete({}); });
                return value_source->create();
            }

        private:
            Async::Scheduler& scheduler;
            Async::TimingPollSource& timing_poll_source;
    };
}