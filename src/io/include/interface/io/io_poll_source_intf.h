#pragma once

#include <cstdio>
#include <functional>

#include "scheduler/poll_source.h"
#include "io/io_request.h"
#include "io/aio_request_result.h"

namespace IO {
    class IPollSource : public Scheduler::IPollSource {
    public:
        using Callback = std::function<void(IO::AIOResult<IO::ReadRequest>)>;
        virtual auto queue_read(FILE* file, IO::ReadRequest request, const Callback& callback) -> void = 0;
    };
}
