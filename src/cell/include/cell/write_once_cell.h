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
    template <typename T, typename Err>
    class WriteOnceCell : public ICell<T, Err> {
    public:
        explicit WriteOnceCell(Scheduler::IScheduler& scheduler);

        [[nodiscard]] auto read() const -> std::optional<Cell::Result<T, Err>> override;

        // error/write performs a concurrent write to the WriteOnceCell
        // it returns a boolean indicating if the error/write was successful (could be written)
        // a false indicates that the cell has already been written to
        // and hence the write was not successful, there exists two overloaded methods
        // for each error/write fn, one method takes no scheduling context so uses an empty scheduling context
        // when dispatching continuations and the other takes a defined context, for more information
        // on scheduling contexts see the documentation under scheduler.h
        auto error(Err err) -> bool { return error(Scheduler::Context::empty(), err); }
        auto error(Scheduler::Context ctx, Err err) -> bool {
            return write_result_to_value(ctx, Cell::Result<T, Err>(err));
        }

        auto write(T write_val) -> bool { return write(Scheduler::Context::empty(), write_val); }
        auto write(Scheduler::Context ctx, T write_val) -> bool {
            return write_result_to_value(ctx, Cell::Result<T, Err>(write_val));
        }

        // await takes a callback function and calls it with the value
        // of the WriteOnceCell when it is available.
        auto await(Callback<T, Err> callback) -> void override;
            
        // block sleeps the current thread until the value of the cell is available
        // it then returns the value of the cell, note that this is different from await
        // as await registers a continuation, while block is a blocking operation
        [[nodiscard]] auto block() const -> Cell::Result<T, Err> override;

    private:
        auto write_result_to_value(Scheduler::Context ctx, Cell::Result<T, Err> result) -> bool;

        mutable std::shared_mutex mutex;
        mutable std::optional<Cell::Result<T, Err>> value;
            
        std::vector<Callback<T, Err>> callbacks;
        mutable std::condition_variable_any cell_filled;

        //  Note: it is an invariant of the Asynchronous library that the scheduler's
        //        is of 'static lifetime and hence will outlive any cell that uses it
        std::reference_wrapper<Scheduler::IScheduler> scheduler;
    };
}






// Implementation
template <typename T, typename Err>
Cell::WriteOnceCell<T, Err>::WriteOnceCell(Scheduler::IScheduler& scheduler) : 
    value(std::nullopt),
    scheduler(scheduler) {}

template <typename T, typename Err>
auto Cell::WriteOnceCell<T, Err>::read() const -> std::optional<Cell::Result<T, Err>> {
    const std::shared_lock lock(mutex);
    return this->value;
}

template <typename T, typename Err>
auto Cell::WriteOnceCell<T, Err>::write_result_to_value(Scheduler::Context ctx, Cell::Result<T, Err> result) -> bool {
    {
        const std::unique_lock lock(mutex);
        if (value.has_value()) { return false; }
        value = std::optional(result);

        // alert callbacks by scheduling continuations
        // on the scheduler
        for (auto& callback : callbacks) {
            scheduler.get().queue(ctx, [=] (auto ctx) { callback(ctx, result); });
        }

        // clear the callbacks to release any reference we may indirectly maintain
        callbacks.clear();
    }

    // awake any blocking threads prior to returning
    cell_filled.notify_all();
    return true;
}

template <typename T, typename Err>
auto Cell::WriteOnceCell<T, Err>::await(Callback<T, Err> callback) -> void {
    // note that we take a shared lock here to prevent lock contention, as 
    // write_result_to_value is the only other fn that would use the callbacks vector (and claims a unique lock)
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

template <typename T, typename Err>
auto Cell::WriteOnceCell<T, Err>::block() const -> Cell::Result<T, Err> {
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