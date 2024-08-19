#pragma once

#include <optional>
#include <functional>

// SchedulingContext carries all information regarding the surrounding information
// around a scheduler job, for now it just contains the worker id that the job
// is to be run on.
namespace Scheduler {
    class Context {
    public:
        friend class JobScheduler;

        [[nodiscard]] static auto empty() -> Context { return Context(); }

        auto operator==(const Context& other) const -> bool {
            return worker_id == other.worker_id;
        }

    private:
        Context() : worker_id(std::nullopt) {}
        Context(int worker_id) : worker_id(worker_id) {}
        std::optional<int> worker_id;
    };
}
