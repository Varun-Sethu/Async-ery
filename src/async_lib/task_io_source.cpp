#include "async_lib/task_io_source.h"


auto Async::TaskIOSource::read(FILE* file, Async::IOReadRequest request) -> Async::Task<Async::IOReadRequest> {
    auto task_source = TaskValueSource<Async::IOReadRequest>(scheduler);
    auto task = task_source.create();
    auto read_callback = [task_source](auto buffer) mutable {
        task_source.complete(buffer);
    };

    io_poll_source.get().queue_read(file, request, read_callback);
    return task;
}