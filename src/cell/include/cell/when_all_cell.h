#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <atomic>

#include "cell.h"
#include "write_once_cell.h"
#include "scheduler/scheduler_intf.h"

namespace Cell {
    // WhenAllCell, like WhenAnyCell assumes ownership of the cells it is tracking
    // and resolves when all of the cells it is tracking resolves
    template <typename T>
    class WhenAllCell : public ICell<std::vector<T>> {
        public:
            WhenAllCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T>>> cells);

            auto read() const -> std::optional<std::vector<T>> override;
            auto await(Callback<std::vector<T>> callback) -> void override;
            auto block() -> std::vector<T> override;

        private:
            struct WhenAllExecutionContext {
                WhenAllExecutionContext(std::vector<T> resolved_values) :
                    resolved_values(resolved_values),
                    num_resolved_cells(0),
                    total_cells(resolved_values.size())
                {}

                std::vector<T> resolved_values;
                std::atomic<size_t> num_resolved_cells;
                size_t total_cells;
            };


            std::shared_ptr<WriteOnceCell<std::vector<T>>> underlying_cell;
            std::vector<std::shared_ptr<ICell<T>>> cells;
    };
}




// Implementation
template <typename T>
Cell::WhenAllCell<T>::WhenAllCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T>>> cells) : 
    underlying_cell(std::make_shared<WriteOnceCell<std::vector<T>>>(scheduler)),
    cells(cells)
{
    // We maintain a shared execution context for the same reason why underlying_cell is itself a shared pointer
    // there is a possibility that the destructor for this cell is invoked prior to the runtime scheduling the continuations
    // for each of the cells, while such a situation is sad we need to ensure no erroneous situations arise, hence we only
    // kill metadata associated with this combinator after all cells are resolved 
    auto execution_context = std::make_shared<WhenAllExecutionContext>(std::vector<T>(cells.size()));

    for (size_t cell_idx = 0; cell_idx < cells.size(); cell_idx += 1) {
        cells[cell_idx]->await([execution_context, cell_idx, underlying_cell=this->underlying_cell](auto ctx, T value) {
            // Safety: we don't actually need mutual exclusion here, the cells array is already resized and we are
            // guaranteed that each cell touches a unique part of the cells array
            auto& [resolved_values, num_resolved_tasks, total_cells] = *execution_context;
            resolved_values[cell_idx] = value;

            // if this was the last task to resolve, we can write the resolved values to the underlying cell
            if (num_resolved_tasks.fetch_add(1, std::memory_order_relaxed) + 1 == total_cells) {
                underlying_cell->write(ctx, resolved_values);
            }
        });
    }
}


template <typename T>
auto Cell::WhenAllCell<T>::read() const -> std::optional<std::vector<T>> { return underlying_cell->read(); }

template <typename T>
auto Cell::WhenAllCell<T>::await(Callback<std::vector<T>> callback) -> void {
    underlying_cell->await(callback);
}

template <typename T>
auto Cell::WhenAllCell<T>::block() -> std::vector<T> { return underlying_cell->block(); }
