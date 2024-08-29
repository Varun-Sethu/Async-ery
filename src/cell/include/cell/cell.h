#pragma once

#include <iostream>
#include <optional>
#include <shared_mutex>
#include <ranges>
#include <functional>
#include <vector>

#include "scheduler/scheduling_context.h"
#include "cell/cell_result.h"

namespace Cell {
    template <typename T, typename Err>
    using Callback = std::function<void(Scheduler::Context, Cell::Result<T, Err>)>;

    template <typename T, typename Err>
    class ICell {
    public:
        virtual auto await(Callback<T, Err> callback) -> void = 0;
        [[nodiscard]] virtual auto read() const -> std::optional<Cell::Result<T, Err>> = 0;
        [[nodiscard]] virtual auto block() const -> Cell::Result<T, Err> = 0;


        virtual ~ICell() = default;
        ICell() = default;
        ICell(ICell&&) = delete;
        ICell(const ICell&) = delete;

        auto operator=(const ICell&) -> ICell& = delete;
        auto operator=(ICell&&) -> ICell& = delete;
    };
}