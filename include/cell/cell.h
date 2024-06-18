#pragma once

#include <iostream>
#include <optional>
#include <shared_mutex>
#include <ranges>
#include <functional>
#include <vector>

namespace Cell {
    template <typename T>
    class ICell {
        public:
            virtual auto read() const -> std::optional<T> = 0;
            virtual auto await(std::function<void(T)> callback) -> void = 0;
            virtual auto block() -> T = 0;
    };
}