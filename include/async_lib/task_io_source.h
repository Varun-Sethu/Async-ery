#pragma once

#include "async_lib/task_value_source.h"
#include "scheduler/scheduler_intf.h"
#include "io/io_poll_source.h"

namespace Async {
    class TaskIOSource {
        public:
            TaskIOSource(Scheduler::IScheduler& scheduler, Async::IOPollSource& io_poll_source) : 
                scheduler(scheduler),
                io_poll_source(io_poll_source) {}


            auto read(FILE* file, Async::IOReadRequest request) -> Async::Task<Async::IOReadRequest>;

        private:
            // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
            //  Note: it is an invariant of the Asynchronous library that the scheduler's
            //        lifetime is longer than the lifetime of any task / cell that uses it.
            //        in the application scope it has a 'static lifetime
            Scheduler::IScheduler& scheduler;
            // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

            // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
            //  Note: it is an invariant of the Asynchronous library that the io_poll_source's
            //        lifetime is longer than TaskIOSource's lifetime
            Async::IOPollSource& io_poll_source;
            // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
    };
}