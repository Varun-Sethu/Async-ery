#pragma once

#include <memory>
#include <utility>
#include <optional>
#include <vector>

#include "cell/cell.h"
#include "cell/cell_result.h"

namespace Cell {
    template<typename T, typename Err>
    class ImmediatelyResolvedCell : public ICell<T, Err> {
    public:
        explicit ImmediatelyResolvedCell(T value);

        // Since this cell resolves at construction, awaiting it will immediately call the callback
        // with the result.
        void await(Callback<T, Err> callback) override;
        
        // Since this cell resolves at construction, reading it will immediately return the result.
        // Ie. by definition it is impossible for this cell to be empty.
        [[nodiscard]] auto read() const -> std::optional<Cell::Result<T, Err>> override;
        [[nodiscard]] auto block() const ->  Cell::Result<T, Err> override;

    private:
        T result_;
    };
}

template<typename T, typename Err>
Cell::ImmediatelyResolvedCell<T, Err>::ImmediatelyResolvedCell(T value)
    : result_(std::move(value)) {}

template<typename T, typename Err>
void Cell::ImmediatelyResolvedCell<T, Err>::await(Callback<T, Err> callback) {
    callback(Scheduler::Context::empty(), { result_ });
}

template<typename T, typename Err>
auto Cell::ImmediatelyResolvedCell<T, Err>::read() const -> std::optional<Cell::Result<T, Err>> {
    return {{ result_ }};
}

template<typename T, typename Err>
auto Cell::ImmediatelyResolvedCell<T, Err>::block() const -> Cell::Result<T, Err> {
    return { result_ };
}
