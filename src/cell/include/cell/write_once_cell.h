#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <shared_mutex>
#include <condition_variable>

#include "cell.h"
#include "scheduler/scheduler_intf.h"

namespace Cell {
    /// WriteOnceCell is a class that represents a cell that can be written to once
    /// and read from multiple times it is thread-safe and can be awaited
    template <typename T>
    class WriteOnceCell : public ICell<T> {
        public:
            explicit WriteOnceCell(Scheduler::IScheduler& scheduler);

            auto read() const -> std::optional<T> override;

            // write performs a concurrent write to the WriteOnceCell
            // it returns a boolean indicating if the write was successful
            // a false indicates that the cell has already been written to
            // and hence the write was not successful, there exists two overloaded methods
            // for this fn, one method takes no scheduling context so uses an empty scheduling context
            // when dispatching continuations and the other takes a defined context, for more information
            // on scheduling contexts see the documentation under scheduler.h
            auto write(T write_val) -> bool { return write(Scheduler::Context::empty(), write_val); }
            auto write(Scheduler::Context ctx, T write_val) -> bool;

            // await takes a callback function and calls it with the value
            // of the WriteOnceCell when it is available.
            auto await(Callback<T> callback) -> void override;
            
            // block sleeps the current thread until the value of the cell is available
            // it then returns the value of the cell, note that this is different from await
            // as await registers a continuation, while block is a blocking operation
            auto block() -> T override;

        private:
            mutable std::shared_mutex mutex;
            mutable std::optional<T> value;
            
            std::vector<Callback<T>> callbacks;
            std::condition_variable_any cell_filled;

            //  Note: it is an invariant of the Asynchronous library that the scheduler's
            //        is of 'static lifetime and hence will outlive any cell that uses it
            std::reference_wrapper<Scheduler::IScheduler> scheduler;
    };
}






// Implementation
template <typename T>
Cell::WriteOnceCell<T>::WriteOnceCell(Scheduler::IScheduler& scheduler) : 
    value(std::nullopt),
    scheduler(scheduler) {}

template <typename T>
auto Cell::WriteOnceCell<T>::read() const -> std::optional<T> {
    const std::shared_lock lock(mutex);
    return this->value;
}

template <typename T>
auto Cell::WriteOnceCell<T>::write(Scheduler::Context ctx, T write_val) -> bool {
    {
        const std::unique_lock lock(mutex);
        if (value.has_value()) { return false; }
        value = std::optional(write_val);

        // alert callbacks by scheduling continuations
        // on the scheduler
        for (auto& callback : callbacks) {
            scheduler.get().queue(ctx, [=] (auto ctx) { callback(ctx, write_val); });
        }

        // clear the callbacks to release any reference we may indirectly maintain
        callbacks.clear();
    }

    // awake any blocking threads prior to returning
    cell_filled.notify_all();
    return true;
}



template <typename T>
auto Cell::WriteOnceCell<T>::await(Callback<T> callback) -> void {
    // note that we take a shared lock here to prevent lock contention, as 
    // write is the only other fn that would use the callbacks vector (and claims a unique lock)
    // it is impossible for both await and write to be in the critical section at the same time
    const std::shared_lock lock(mutex);
    if (value.has_value()) {
        auto value_inner = value.value();
        scheduler.get().queue(Scheduler::Context::empty(),
                        [callback, value_inner] (auto ctx) { callback(ctx, value_inner); });
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

    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    // Note: it's safe to perform an unchecked optional access here as the semantics of the 
    //       cell_filled.wait function guarantees that the cell will be filled by the time we reach this point
    //       hence the optional will never be nullopt
    return value.value();
    // NOLINTEND(bugprone-unchecked-optional-access)
}