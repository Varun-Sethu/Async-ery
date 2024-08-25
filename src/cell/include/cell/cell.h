#pragma once

#include <iostream>
#include <optional>
#include <shared_mutex>
#include <ranges>
#include <functional>
#include <vector>

#include "scheduler/scheduling_context.h"

namespace Cell {
    template <typename T>
    using Callback = std::function<void(Scheduler::Context, T)>;

    template <typename T>
    class ICell {
    public:
        virtual auto await(Callback<T> callback) -> void = 0;
        [[nodiscard]] virtual auto read() const -> std::optional<T> = 0;
        [[nodiscard]] virtual auto block() const -> T = 0;


        virtual ~ICell() = default;
        ICell() = default;
        ICell(ICell&&) = delete;
        ICell(const ICell&) = delete;

        auto operator=(const ICell&) -> ICell& = delete;
        auto operator=(ICell&&) -> ICell& = delete;
    };
}