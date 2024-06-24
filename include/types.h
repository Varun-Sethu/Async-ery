#pragma once

#include <variant>
#include <functional>

namespace Async {
    using job = std::function<void(void)>;
    using unit = std::monostate;

    // TODO: Incorporate this into the task return type
    // such that we can thread errors through tasks
    template <typename T>
    using Resolved = T;
    enum Status {
        Pending,
        Rejected
    };

    template <typename T>
    using Result = std::variant<Resolved<T>, Status>;
};


namespace SchedulerTypes {
    // Job in the context of a scheduler is distinct from
    // the job in the context of an async fn, this is because
    // async jobs do not return anything whereas scheduler jobs
    // require a scheduler context for queueing continuations onto their owner worker
    // threads
    using Context = std::optional<int>;
    using Job = std::function<void(Context)>;
};