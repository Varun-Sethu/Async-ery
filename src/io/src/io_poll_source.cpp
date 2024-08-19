#include "io/io_poll_source.h"

auto Async::IOPollSource::poll() -> std::vector<Scheduler::Job> {                
    auto lock = std::lock_guard<SpinLock>(spinlock);
    auto completed_jobs = std::vector<Scheduler::Job>();
    auto pending_requests = std::vector<std::pair<Callback, InFlightAIORequest>>();

    for (auto& [callback, request] : in_flight_requests) {
        if (request.is_completed()) {
            completed_jobs.push_back([callback, request](auto ctx) mutable {
                auto underlying = request.underlying_request();
                callback(underlying);
            });
        } else {
            pending_requests.push_back({ callback, std::move(request) });
        }
    }

    // evict all completed jobs from the in flight requests
    in_flight_requests = std::move(pending_requests);
    return completed_jobs;
};


auto Async::IOPollSource::queue_read(FILE* fp, Async::IOReadRequest request, Callback callback) -> void {
    auto in_flight_request = AIOManager::enqueue_and_start_read(fp, request);
    auto lock = std::lock_guard<SpinLock>(spinlock);
    in_flight_requests.push_back({ callback, std::move(in_flight_request) });
}