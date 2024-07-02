#pragma once

#include <optional>
#include <functional>

namespace Async {
    // SchedulingContext carries all information regarding the surrounding information
    // around a scheduler job, for now it just contains the worker id that the job
    // is to be run on.
    class SchedulingContext {
        friend class Scheduler;
        public:
            auto operator==(const SchedulingContext& other) const -> bool {
                return worker_id == other.worker_id;
            }

            static auto empty() -> SchedulingContext { return SchedulingContext(); }

        private:
            SchedulingContext() : worker_id(std::nullopt) {}
            SchedulingContext(int worker_id) : worker_id(worker_id) {}
            std::optional<int> worker_id;
    };

    using SchedulerJob = std::function<void(SchedulingContext)>;
};