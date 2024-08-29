#pragma once

#include <shared_mutex>
#include <optional>
#include <vector>
#include <condition_variable>
#include <functional>

#include "scheduler/scheduler_intf.h"
#include "cell.h"

namespace Cell {
    /// TrackingOnceCell is a class that represents a cell that tracks another cell
    /// it is a read-only cell that can be awaited, and reads from the cell it is tracking
    /// you can specify what cell is being tracked post instantiation, note that a cell
    /// to track can only be specified once
    template <typename T, typename Err>
    class TrackingOnceCell : public ICell<T, Err> {
    public:
        [[nodiscard]] auto read() const -> std::optional<Cell::Result<T, Err>> override;

        // await takes a callback function and calls it with the value
        // of the cell being tracked when it is available, if no cell 
        // is being tracked, the callback is added to a list of callbacks
        // and added once we have a tracking cell
        auto await(Callback<T, Err> callback) -> void override;

        // track sets the cell to track, if a cell is already being tracked
        // the function returns false, otherwise it returns true indicating a successful track
        // attempt
        auto track(std::shared_ptr<ICell<T, Err>> new_cell) -> bool;

        // block sleeps the current thread until the value of the cell is available
        // it then returns the value of the cell, note that this is different from await
        // as await registers a continuation, while block is a blocking operation
        [[nodiscard]] auto block() const -> Cell::Result<T, Err> override;

    private:
        // A note on callbacks:
        // we need to maintain a set of callbacks for the TrackingCell
        // as we may not know what cell we are tracking until much later, hence we need to
        // maintain a set of subscribers to fill once we have a cell to track
        std::optional<std::shared_ptr<ICell<T, Err>>> cell;
        std::vector<Callback<T, Err>> callbacks;

        mutable std::condition_variable_any cell_filled;
        mutable std::shared_mutex mutex;
    };
}





template <typename T, typename Err>
auto Cell::TrackingOnceCell<T, Err>::read() const -> std::optional<Cell::Result<T, Err>> {
    const std::shared_lock lock(mutex);
    return cell.has_value() 
                ? cell.value()->read()
                : std::nullopt;
}


template <typename T, typename Err>
auto Cell::TrackingOnceCell<T, Err>::await(Callback<T, Err> callback) -> void {
    // note that we take a shared lock here to prevent lock contention, as 
    // track is the only other fn that would use the callbacks vector (and claims a unique lock)
    // it is impossible for both await and write to be in the critical section at the same time
    const std::shared_lock lock(mutex);
    if (!cell.has_value()) {
        callbacks.push_back(callback);
        return;
    }

    cell.value()->await(callback);
}


template <typename T, typename Err>
auto Cell::TrackingOnceCell<T, Err>::track(std::shared_ptr<ICell<T, Err>> new_cell) -> bool {
    {
        const std::unique_lock lock(mutex);
        if (cell.has_value()) { return false; }
    
        cell = std::optional(new_cell);
    
        // alert callbacks by registering them as callbacks
        // on the underlying cell
        for (auto& callback : callbacks) {
            cell.value()->await(callback);
        }
    
        callbacks.clear();
    }

    // raise the fact that the cell is filled is now true, releasing any threads waiting on this condition
    // variable
    this->cell_filled.notify_all();
    return true;
}


template <typename T, typename Err>
auto Cell::TrackingOnceCell<T, Err>::block() const -> Cell::Result<T, Err> {
    std::shared_lock lock(mutex);

    if (!this->cell.has_value()) { cell_filled.wait(lock); }
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    // Note: it's safe to perform an unchecked optional access here as the semantics of the 
    //       cell_filled.wait function guarantees that the cell will be filled by the time we reach this point
    //       hence the optional will never be nullopt
    return this->cell.value()->block();
    // NOLINTEND(bugprone-unchecked-optional-access)
}