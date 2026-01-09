#pragma once

#include <queue>

#include "scheduler/scheduler_intf.h"

namespace Mocks {
    class MockScheduler : public Scheduler::IScheduler {
    public:
        auto queue(Scheduler::Context ctx, Scheduler::Job&& job_fn) -> void override {
            jobs_.push({ctx, std::move(job_fn)});
        }

        auto run_next_job() -> bool {
            if (jobs_.empty()) {
                return false;
            }

            auto [ctx, job] = std::move(jobs_.front());
            jobs_.pop();
            job(ctx);
            return true;
        }

        auto run_all_jobs() -> void {
            while (run_next_job()) {}
        }

        [[nodiscard]] auto pending_job_count() const -> size_t {
            return jobs_.size();
        }

    private:
        struct QueuedJob {
            Scheduler::Context ctx;
            Scheduler::Job job;
        };

        std::queue<QueuedJob> jobs_;
    };
}
