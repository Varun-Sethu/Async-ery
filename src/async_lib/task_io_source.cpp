#include <cstdio>
#include <utility>

#include "async_lib/task_io_source.h"
#include "async_lib/task.h"
#include "async_lib/task_value_source.h"
#include "io/io_request.h"
#include "async_lib/async_result.h"
#include "io/aio_request_result.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNUSED(x) __attribute__((unused))x
// NOLINTEND(cppcoreguidelines-macro-usage)


auto Async::TaskIOSource::read(FILE* file, IO::ReadRequest request) -> Async::Task<IO::ReadRequest> {
    auto task_source = TaskValueSource<IO::ReadRequest>(scheduler);
    auto task = task_source.create();
    auto read_callback = [task_source](auto io_result) mutable {
        IO::visit_aio_result(io_result, 
            [&task_source](const IO::ReadRequest& buff) { task_source.complete(buff); },
            [&task_source](UNUSED(auto err)) { task_source.error(Async::IOError); }
        );
    };

    io_poll_source.get().queue_read(file, std::move(request), read_callback);
    return task;
}