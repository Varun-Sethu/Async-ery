#pragma once

#include <vector>
#include <algorithm>

#include "interface/timing/timer_poll_source_intf.h"

namespace Mocks {
    class MockTimerPollSource : public Timing::IPollSource {
    public:
        auto poll_frequency() -> std::chrono::milliseconds override {
            return std::chrono::milliseconds(1);
        }

        auto poll() -> std::vector<Scheduler::Job> override {
            std::vector<Scheduler::Job> ready_jobs;

            auto it = scheduled_timers_.begin();
            while (it != scheduled_timers_.end()) {
                if (it->expiry <= current_time_) {
                    ready_jobs.push_back(std::move(it->job));
                    it = scheduled_timers_.erase(it);
                } else {
                    ++it;
                }
            }

            return ready_jobs;
        }

        auto schedule(std::chrono::milliseconds expiry, Scheduler::Job task) -> void override {
            scheduled_timers_.push_back({current_time_ + expiry, std::move(task)});
        }

        auto advance_time(std::chrono::milliseconds duration) -> std::vector<Scheduler::Job> {
            current_time_ += duration;
            return poll();
        }

        [[nodiscard]] auto current_time() const -> std::chrono::milliseconds {
            return current_time_;
        }

        [[nodiscard]] auto pending_timer_count() const -> size_t {
            return scheduled_timers_.size();
        }

    private:
        struct ScheduledTimer {
            std::chrono::milliseconds expiry;
            Scheduler::Job job;
        };

        std::chrono::milliseconds current_time_{0};
        std::vector<ScheduledTimer> scheduled_timers_;
    };
}
