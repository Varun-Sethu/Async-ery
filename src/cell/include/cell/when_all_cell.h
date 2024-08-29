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
    template <typename T, typename Err>
    class WhenAllCell : public ICell<std::vector<T>, Err> {
    public:
        WhenAllCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T, Err>>> cells);

        [[nodiscard]] auto read() const -> std::optional<Cell::Result<std::vector<T>, Err>> override;
        [[nodiscard]] auto block() const -> Cell::Result<std::vector<T>, Err> override;
        auto await(Callback<std::vector<T>, Err> callback) -> void override;

    private:
        struct WhenAllExecutionContext {
        public:
            explicit WhenAllExecutionContext(std::vector<T> resolved_values) :
                _resolved_values(resolved_values),
                _num_resolved_cells(0),
                _total_cells(resolved_values.size())
            {}

            // commit_resolved_value is a helper function that commits a resolved value to the execution context
            // and increments the number of resolved cells. The boolean returned indicates if committing this value
            // resulted in ALL cells having finally been resolved
            // Safety: we don't actually need mutual exclusion here, the cells array is already resized and we are
            // guaranteed that each cell touches a unique part of the cells array
            [[nodiscard]] auto commit_resolved_value(size_t cell_id, T value) -> bool {
                _resolved_values[cell_id] = value;
                auto cells_resolved_so_far = _num_resolved_cells.fetch_add(1, std::memory_order_relaxed);
                return cells_resolved_so_far + 1 >= _total_cells;
            }

            [[nodiscard]] auto resolved_values() -> std::vector<T>& { return _resolved_values; }

        private:
            std::vector<T> _resolved_values;
            std::atomic<size_t> _num_resolved_cells;
            size_t _total_cells;
        };


        std::shared_ptr<WriteOnceCell<std::vector<T>, Err>> underlying_cell;
        std::vector<std::shared_ptr<ICell<T, Err>>> cells;
    };
}




// Implementation
template <typename T, typename Err>
Cell::WhenAllCell<T, Err>::WhenAllCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T, Err>>> cells) : 
    underlying_cell(std::make_shared<WriteOnceCell<std::vector<T>, Err>>(scheduler)),
    cells(std::move(cells))
{
    // We maintain a shared execution context for the same reason why underlying_cell is itself a shared pointer
    // there is a possibility that the destructor for this cell is invoked prior to the runtime scheduling the continuations
    // for each of the cells, while such a situation is sad we need to ensure no erroneous situations arise, hence we only
    // kill metadata associated with this combinator after all cells are resolved 
    auto execution_context = std::make_shared<WhenAllExecutionContext>(std::vector<T>(this->cells.size()));
    auto underlying_cell = this->underlying_cell;

    for (auto cell_id = size_t(0); cell_id < this->cells.size(); cell_id += 1) {
        this->cells[cell_id]->await([execution_context, cell_id, underlying_cell](auto ctx, auto value) {
            Cell::visit_result(value, 
                [execution_context, cell_id, underlying_cell, ctx](T value) {
                    auto all_cells_resolved = execution_context->commit_resolved_value(cell_id, value);
                    if (all_cells_resolved) {
                        underlying_cell->write(ctx, execution_context->resolved_values());
                    }
                },
                [execution_context, underlying_cell, ctx](Err err) { underlying_cell->error(ctx, err);}
            );            
        });
    }
}


template <typename T, typename Err>
auto Cell::WhenAllCell<T, Err>::read() const -> std::optional<Cell::Result<std::vector<T>, Err>> { return underlying_cell->read(); }

template <typename T, typename Err>
auto Cell::WhenAllCell<T, Err>::await(Callback<std::vector<T>, Err> callback) -> void {
    underlying_cell->await(callback);
}

template <typename T, typename Err>
auto Cell::WhenAllCell<T, Err>::block() const -> Cell::Result<std::vector<T>, Err> { return underlying_cell->block(); }
