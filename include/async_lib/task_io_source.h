#pragma once

#include "async_lib/task_value_source.h"
#include "scheduler/scheduler_intf.h"
#include "io/io_poll_source.h"

namespace Async {
    class TaskIOSource {
        public:
            TaskIOSource(Scheduler::IScheduler& scheduler, IO::PollSource& io_poll_source) : 
                scheduler(scheduler),
                io_poll_source(io_poll_source) {}


            auto read(FILE* file, IO::ReadRequest request) -> Async::Task<IO::ReadRequest>;

        private:
            // Note:
            //      It is expected that the lifetime of the scheduler is longer than the lifetime of the TaskIOSource
            //      the scheduler's lifetime should match the entire application lifetime.
            //
            //      It is expected that the lifetime of the io_poll_source is longer than the lifetime of the TaskIOSource
            //      the io_poll_source's lifetime should match the entire application lifetime. This is trivially true as
            //      the io_poll_source is captured via shared ownership by the scheduler, hence its lifetime is the same
            std::reference_wrapper<Scheduler::IScheduler> scheduler;
            std::reference_wrapper<IO::PollSource> io_poll_source;
    };
}