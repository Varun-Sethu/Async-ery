#pragma once

#include <variant>
#include <functional>

namespace Async {
    using Job = std::function<void(void)>;
    using Unit = std::monostate;

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