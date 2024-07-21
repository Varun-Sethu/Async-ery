#pragma once

#include "task_value_source.h"
#include "scheduler/scheduler.h"
#include "polling/io_poll_source.h"

namespace Async {
    class TaskIOSource {
        public:
            TaskIOSource(Async::Scheduler& scheduler, Async::IOPollSource& io_poll_source) : 
                scheduler(scheduler),
                io_poll_source(io_poll_source) {}


            auto read(FILE* fp, Async::IOReadRequest request) -> Async::Task<Async::IOReadRequest> {
                auto task_source = TaskValueSource<Async::IOReadRequest>(scheduler);
                auto task = task_source.create();
                auto read_callback = [task_source](auto buffer) mutable {
                    task_source.complete(buffer);
                };

                io_poll_source.queue_read(fp, request, read_callback);
                return task;
            }

        private:
            Async::Scheduler& scheduler;
            Async::IOPollSource& io_poll_source;
    };
}