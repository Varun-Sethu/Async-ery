#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <functional>

#include "cell.h"
#include "write_once_cell.h"
#include "scheduler/scheduler_intf.h"

namespace Cell {
    // WhenAnyCell assumes ownership of the cells it is tracking
    // and resolves when any of the cells it is tracking resolves
    template <typename T, typename Err>
    class WhenAnyCell : public ICell<T, Err> {
    public:
        WhenAnyCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T, Err>>> cells);

        [[nodiscard]] auto read() const -> std::optional<Cell::Result<T, Err>> override;
        [[nodiscard]] auto block() const -> Cell::Result<T, Err> override;
        auto await(Callback<T, Err> callback) -> void override;
    
    private:
        class WhenAnyExecutionContext {
        public:
            explicit WhenAnyExecutionContext(size_t num_cells) : _num_erroneous_cells(0), total_cells(num_cells) {}

            // log_error commits an error to the execution context's error log, if all the cells have errored out
            // then this function returns true, otherwise it returns false
            [[nodiscard]] auto log_error() -> bool {
                auto cells_errored_so_far = 1 + _num_erroneous_cells.fetch_add(1, std::memory_order_relaxed);
                return cells_errored_so_far >= total_cells;
            }

        private:
            std::atomic<size_t> _num_erroneous_cells;
            size_t total_cells;
        };

        std::shared_ptr<WriteOnceCell<T, Err>> underlying_cell;
        std::vector<std::shared_ptr<ICell<T, Err>>> cells;
    };
}



// Implementation
template <typename T, typename Err>
Cell::WhenAnyCell<T, Err>::WhenAnyCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T, Err>>> cells) : 
    underlying_cell(std::make_shared<WriteOnceCell<T, Err>>(scheduler)),
    cells(std::move(cells))
{
    // Underlying cell must be a shared pointer because there is a chance that the destructor of the WhenAnyCell
    // gets invoked prior to the runtime scheduling the continuation we pass to the cells... if this happens it would be sad
    // however to prevent erroneous situations we only invoke the destructor of the underlying cell once all the cells have resolved
    // ie. each of the callbacks must assume ownership of the underlying cell
    auto exe_ctx = std::make_shared<WhenAnyExecutionContext>(this->cells.size());
    auto underlying_cell = this->underlying_cell;

    for (auto& cell: this->cells) {
        cell->await([underlying_cell, exe_ctx](auto ctx, Cell::Result<T, Err> value) {
            Cell::visit_result(value, 
                [underlying_cell, ctx](T value) { underlying_cell->write(ctx, value); }, 
                [underlying_cell, ctx, exe_ctx] (Err err) {
                    auto all_cells_have_errored = exe_ctx->log_error();
                    if (all_cells_have_errored) {
                        underlying_cell->error(ctx, err);
                    }
                }
            );
        });
    }
}


template <typename T, typename Err>
auto Cell::WhenAnyCell<T, Err>::read() const -> std::optional<Cell::Result<T, Err>> { return underlying_cell->read(); }

template <typename T, typename Err>
auto Cell::WhenAnyCell<T, Err>::await(Callback<T, Err> callback) -> void { underlying_cell->await(callback); }

template <typename T, typename Err>
auto Cell::WhenAnyCell<T, Err>::block() const -> Cell::Result<T, Err> { return underlying_cell->block(); }