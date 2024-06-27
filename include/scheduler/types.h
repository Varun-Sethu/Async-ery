#pragma once

#include <optional>
#include <functional>

namespace Async {
    // SchedulingContext carries all information regarding the surrounding information
    // around a scheduler job, for now it just contains the worker id that the job
    // is to be run on.
    class SchedulingContext {
        friend class Scheduler;
        private:
            SchedulingContext() : worker_id(std::nullopt) {}
            SchedulingContext(int worker_id) : worker_id(worker_id) {}
            std::optional<int> worker_id;
    };

    using SchedulerJob = std::function<void(SchedulingContext)>;
};