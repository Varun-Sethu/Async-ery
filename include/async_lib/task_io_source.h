#pragma once

#include "async_lib/task_value_source.h"
#include "scheduler/scheduler.h"
#include "io/io_poll_source.h"

namespace Async {
    class TaskIOSource {
        public:
            TaskIOSource(Async::Scheduler& scheduler, Async::IOPollSource& io_poll_source) : 
                scheduler(scheduler),
                io_poll_source(io_poll_source) {}


            auto read(FILE* fp, Async::IOReadRequest request) -> Async::Task<Async::IOReadRequest>;

        private:
            Async::Scheduler& scheduler;
            Async::IOPollSource& io_poll_source;
    };
}