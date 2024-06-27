#pragma once

#include <iostream>
#include <optional>
#include <shared_mutex>
#include <ranges>
#include <functional>
#include <vector>

#include "scheduler/types.h"

namespace Cell {
    template <typename T>
    using Callback = std::function<void(Async::SchedulingContext, T)>;

    template <typename T>
    class ICell {
        public:
            virtual auto read() const -> std::optional<T> = 0;
            virtual auto await(Callback<T> callback) -> void = 0;
            virtual auto block() -> T = 0;
    };
}