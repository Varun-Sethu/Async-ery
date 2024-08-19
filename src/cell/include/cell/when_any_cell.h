#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "cell.h"
#include "write_once_cell.h"
#include "scheduler/scheduler_intf.h"

namespace Cell {
    // WhenAnyCell assumes ownership of the cells it is tracking
    // and resolves when any of the cells it is tracking resolves
    template <typename T>
    class WhenAnyCell : public ICell<T> {
        public:
            WhenAnyCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T>>> cells);

            auto read() const -> std::optional<T> override;
            auto await(Callback<T> callback) -> void override;
            auto block() -> T override;
    
        private:
            std::shared_ptr<WriteOnceCell<T>> underlying_cell;
            std::vector<std::shared_ptr<ICell<T>>> cells;
    };
}



// Implementation
template <typename T>
Cell::WhenAnyCell<T>::WhenAnyCell(Scheduler::IScheduler& scheduler, std::vector<std::shared_ptr<ICell<T>>> cells) : 
    underlying_cell(std::make_shared<WriteOnceCell<T>>(scheduler)),
    cells(cells)
{
    // Underlying cell must be a shared pointer because there is a chance that the destructor of the WhenAnyCell
    // gets invoked prior to the runtime scheduling the continuation we pass to the cells... if this happens it would be sad
    // however to prevent erroneous situations we only invoke the destructor of the underlying cell once all the cells have resolved
    for (auto& cell : cells) {
        cell->await([underlying_cell=this->underlying_cell](auto ctx, T value) {
            underlying_cell->write(ctx, value);
        });
    }
}


template <typename T>
auto Cell::WhenAnyCell<T>::read() const -> std::optional<T> { return underlying_cell->read(); }

template <typename T>
auto Cell::WhenAnyCell<T>::await(Callback<T> callback) -> void { underlying_cell->await(callback); }

template <typename T>
auto Cell::WhenAnyCell<T>::block() -> T { return underlying_cell->block(); }