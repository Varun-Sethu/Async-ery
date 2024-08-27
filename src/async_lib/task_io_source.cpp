#include <cstdio>
#include <utility>

#include "async_lib/task_io_source.h"
#include "async_lib/task.h"
#include "async_lib/task_value_source.h"
#include "io/io_request.h"


auto Async::TaskIOSource::read(FILE* file, IO::ReadRequest request) -> Async::Task<IO::ReadRequest> {
    auto task_source = TaskValueSource<IO::ReadRequest>(scheduler);
    auto task = task_source.create();
    auto read_callback = [task_source](auto buffer) mutable {
        task_source.complete(buffer);
    };

    io_poll_source.get().queue_read(file, std::move(request), read_callback);
    return task;
}