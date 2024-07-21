#pragma once

#include <vector>
#include <chrono>
#include <memory>
#include <linux/aio_abi.h>
#include <mutex>
#include <aio.h>

#include "util/aio.h"
#include "util/spinlock.h"
#include "io_request.h"

using std::chrono_literals::operator""ms;

// NOTE: This class repeatedly polls the state of various IO jobs via a syscall
// this is probably not ideal and in the future should be upgraded to use a more
// efficient mechanism, aio allows us to suspend a thread until a tracked job is complete
// perhaps we can use this to avoid polling
namespace Async {    
    class IOPollSource : public Async::IPollSource {
        using Callback = std::function<void(Async::IOReadRequest)>;

        public:
            auto poll_frequency() -> std::chrono::milliseconds override { return 5ms; };

            auto poll() -> std::vector<SchedulerJob> override {                
                auto lock = std::lock_guard<SpinLock>(spinlock);
                auto completed_jobs = std::vector<SchedulerJob>();
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

            auto queue_read(FILE* fp, Async::IOReadRequest request, Callback callback) -> void {
                auto in_flight_request = AIOManager::enqueue_and_start_read(fp, request);
                auto lock = std::lock_guard<SpinLock>(spinlock);
                in_flight_requests.push_back({ callback, std::move(in_flight_request) });
            }

        private:
            SpinLock spinlock;
            std::vector<std::pair<Callback, InFlightAIORequest>> in_flight_requests; 
    };
}