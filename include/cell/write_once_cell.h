#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <shared_mutex>
#include <condition_variable>

#include "cell.h"
#include "scheduler.h"

namespace Cell {
    /// WriteOnceCell is a class that represents a cell that can be written to once
    /// and read from multiple times it is thread-safe and can be awaited
    template <typename T>
    class WriteOnceCell : public ICell<T> {
        public:
            WriteOnceCell(Async::Scheduler& scheduler);

            auto read() const -> std::optional<T> override;

            // write performs a concurrent write to the WriteOnceCell
            // it returns a boolean indicating if the write was successful
            // a false indicates that the cell has already been written to
            // and hence the write was not successful
            auto write(T write_val) -> bool;

            // await takes a callback function and calls it with the value
            // of the WriteOnceCell when it is available.
            auto await(std::function<void(T)> callback) -> void override;
            
            // block sleeps the current thread until the value of the cell is available
            // it then returns the value of the cell, note that this is different from await
            // as await registers a continuation, while block is a blocking operation
            auto block() -> T override;

        private:
            mutable std::shared_mutex mutex;
            
            std::optional<T> value;
            std::vector<std::function<void(T)>> callbacks;
            std::condition_variable_any cell_filled;

            Async::Scheduler& scheduler;
    };
};






// Implementation
template <typename T>
Cell::WriteOnceCell<T>::WriteOnceCell(Async::Scheduler& scheduler) : scheduler(scheduler) {}

template <typename T>
auto Cell::WriteOnceCell<T>::read() const -> std::optional<T> {
    std::shared_lock lock(mutex);
    return value;
}



template <typename T>
auto Cell::WriteOnceCell<T>::write(T write_val) -> bool {
    {
        std::unique_lock lock(mutex);
        if (value.has_value()) { return false; }
        value = std::optional(write_val);

        // alert callbacks
        for (auto& callback : callbacks) {
            scheduler.queue([=] () { callback(write_val); });
        }

        // clear the callbacks to release any reference we may indirectly maintain
        callbacks.clear();
    }

    // awake any blocking threads prior to returning
    cell_filled.notify_all();
    return true;
}



template <typename T>
auto Cell::WriteOnceCell<T>::await(std::function<void(T)> callback) -> void {
    // note that we take a shared lock here to prevent lock contention, as 
    // write is the only other fn that would use the callbacks vector (and claims a unique lock)
    // it is impossible for both await and write to be in the critical section at the same time
    std::shared_lock lock(mutex);
    if (value.has_value()) {
        auto value_inner = value.value();
        scheduler.queue([=] () { callback(value_inner); });
        return;
    }

    callbacks.push_back(callback);
}

template <typename T>
auto Cell::WriteOnceCell<T>::block() -> T {
    {
        std::shared_lock lock(mutex);
        if (!value.has_value()) {
            cell_filled.wait(lock);
        }
    }

    return value.value();
}