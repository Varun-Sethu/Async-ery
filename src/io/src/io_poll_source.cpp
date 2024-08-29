#include <mutex>
#include <vector>
#include <utility>
#include <cstdio>

#include "io/io_poll_source.h"
#include "io/io_request.h"
#include "io/aio.h"
#include "concurrency/spinlock.h"
#include "scheduler/job.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNUSED(x) __attribute__((unused))x
// NOLINTEND(cppcoreguidelines-macro-usage)

auto IO::PollSource::poll() -> std::vector<Scheduler::Job> {                
    const auto lock = std::lock_guard<SpinLock>(spinlock);
    auto completed_jobs = std::vector<Scheduler::Job>();
    auto pending_requests = std::vector<std::pair<Callback, InFlightAIORequest>>();

    for (auto& [callback, request] : in_flight_requests) {
        if (request.is_completed()) {
            completed_jobs.emplace_back([callback=callback, request = std::move(request)](UNUSED(auto ctx)) {
                auto underlying = request.result();
                callback(underlying);
            });
        } else {
            pending_requests.emplace_back(callback, std::move(request));
        }
    }

    // evict all completed jobs from the in flight requests
    in_flight_requests = std::move(pending_requests);
    return completed_jobs;
};


auto IO::PollSource::queue_read(FILE* file, IO::ReadRequest request, const Callback& callback) -> void {
    auto in_flight_request = AIOManager::enqueue_and_start_read(file, std::move(request));

    const auto lock = std::lock_guard<SpinLock>(spinlock);
    in_flight_requests.emplace_back(callback, std::move(in_flight_request));
}