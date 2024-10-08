#pragma once

#include <vector>
#include <chrono>
#include <memory>
#include <mutex>

#include "aio.h"
#include "io_request.h"
#include "aio_request_result.h"
#include "scheduler/poll_source.h"
#include "concurrency/spinlock.h"

using std::chrono_literals::operator""ms;

// NOTE: This class repeatedly polls the state of various IO jobs via a syscall
// this is probably not ideal and in the future should be upgraded to use a more
// efficient mechanism, aio allows us to suspend a thread until a tracked job is complete
// perhaps we can use this to avoid polling
namespace IO {
    class PollSource : public Scheduler::IPollSource {
    public:
        using Callback = std::function<void(IO::AIOResult<IO::ReadRequest>)>;
        auto poll_frequency() -> std::chrono::milliseconds override { return 5ms; };
        auto poll() -> std::vector<Scheduler::Job> override;
        auto queue_read(FILE* file, IO::ReadRequest request, const Callback& callback) -> void;

    private:
        SpinLock spinlock;
        std::vector<std::pair<Callback, InFlightAIORequest>> in_flight_requests;
    };
}