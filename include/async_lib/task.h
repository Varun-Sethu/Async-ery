#pragma once

#include <iostream>
#include <functional>
#include <memory>

#include "async_lib/types.h"
#include "async_lib/async_result.h"
#include "scheduler/scheduler_intf.h"
#include "cell/write_once_cell.h"
#include "cell/tracking_once_cell.h"
#include "cell/when_any_cell.h"
#include "cell/when_all_cell.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UNUSED(x) __attribute__((unused))x
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace Async {
    template <typename T>
    class TaskValueSource;  // see comment for TaskValueSource in task_value_source.h

    // Task is a class that represents a task that can be awaited
    // it is simply just a wrapper around a IReadableCell and prevents direct writes to the cell
    // Abstracting over direct writes to a cell allows multiple tasks to be driven by the same underlying
    // cell SAFELY
    template <typename T>
    class Task {
    private:
        template <typename Q> friend class Task;
        friend class TaskValueSource<T>;   // for exposing private Task constructor that takes a cell

    public:
        Task(Scheduler::IScheduler& scheduler, std::function<T(void)> func);

        // bind is a method that takes a function that takes the value of the cell and returns a new task
        // it then returns a new task that will resolve to the value of the new task
        template <typename G>
        [[nodiscard]] auto bind(std::function<Task<G>(T)> func) -> Task<G>;

        // map is a method that takes a function and applies it to the value of the cell
        // it then returns a new task that will resolve to the result of the function
        template <typename G>
        [[nodiscard]] auto map(std::function<G(T)> func) -> Task<G>;

        // block will pause the current thread until the value of the cell is available
        // it will then return the value of the cell.
        [[nodiscard]] auto block() -> Async::Result<T>;

        // when_any is a task that resolves when any of the provided underlying tasks are resolved
        // under the hood the task claims shared ownership of the cells it is tracking
        [[nodiscard]] static auto when_any(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<T>;

        // when_all resolves when all the underlying tasks have resolved, returning the values of the cells
        // in a final vector whose order matches the order of the tasks
        [[nodiscard]] static auto when_all(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<std::vector<T>>;

    protected:
        // ICell are an implementation detail so creation of Tasks from them is restricted
        // to be exclusively a private constructor
        Task(Scheduler::IScheduler& scheduler, std::shared_ptr<Cell::ICell<T, Async::Error>> cell) : 
            scheduler(scheduler), cell(std::move(cell)) {}
        
    private:
        static auto task_list_to_cell_list(std::vector<Task<T>> tasks) -> std::vector<std::shared_ptr<Cell::ICell<T, Async::Error>>>;

        //  Note: it is an invariant of the Asynchronous library that the scheduler's
        //        lifetime is longer than the lifetime of any task / cell that uses it.
        //        in the application scope it has a 'static lifetime
        std::reference_wrapper<Scheduler::IScheduler> scheduler;
        std::shared_ptr<Cell::ICell<T, Async::Error>> cell;
    };
}






// Implementation
template <typename T>
Async::Task<T>::Task(Scheduler::IScheduler& scheduler, std::function<T(void)> func) : scheduler(scheduler) {
    auto cell = std::make_shared<Cell::WriteOnceCell<T, Async::Error>>(scheduler);
    this->cell = cell;
    this->scheduler.get().queue(
        Scheduler::Context::empty(),
        [cell, func](auto ctx) {
            auto result = func();
            cell->write(ctx, result);
        }
    );
}


// A subtle point on correctness:
// consider the following method chain:
//      auto computation = task
//          .map<int>([](int value) { return value + 5; })
//          .map<int>([](int value) { return 2 * value + 5; })
//          .map<int>([](int value) { 
//              std::cout << value * 2 << std::endl; 
//              return 3;
//          });
//
// It is tricky to reason about the lifetimes of the individual intermediate tasks...
// each task lives for the duration of the expression itself, each invocation to 
// cell->await will actually capture a shared_ptr to the cell, itself, the implication
// of this is that all parent tasks maintain references to their child (CELLS), hence we 
// do not run in the subtle issue of missing updates as the bottom post child as all the
// intermediate child cells are still alive
template <typename T>
template <typename G>
auto Async::Task<T>::bind(std::function<Task<G>(T)> func) -> Task<G> {
    auto tracking_cell = std::make_shared<Cell::TrackingOnceCell<G, Async::Error>>();
    auto error_cell = std::make_shared<Cell::WriteOnceCell<G, Async::Error>>(scheduler);

    auto callback = [tracking_cell, error_cell, func](auto ctx, Cell::Result<T, Async::Error> value) {
        auto cell_to_track = Cell::map_result(value, 
            [func](T value) { return func(value).cell; },
            [ctx, error_cell](Async::Error err) { 
                error_cell->error(ctx, err);
                return std::static_pointer_cast<Cell::ICell<G, Async::Error>>(error_cell);
            }
        );

        tracking_cell->track(cell_to_track);
    };

    this->cell->await(callback);
    return { scheduler, tracking_cell };
}


// map can be implemented rather simply as a invocation of bind
// however for efficiency reasons we do not do this and instead 
// implement map using a WORM cell
template <typename T>
template <typename G>
auto Async::Task<T>::map(std::function<G(T)> func) -> Task<G> {
    auto cell = std::make_shared<Cell::WriteOnceCell<G, Async::Error>>(scheduler);
    auto callback = [cell, func](auto ctx, Cell::Result<T, Async::Error> value) {
        Cell::visit_result(value, 
            [cell, func, ctx](T value) { cell->write(ctx, func(value)); },
            [cell, ctx](Async::Error err) { cell->error(ctx, err); });
    };

    this->cell->await(callback);
    return { scheduler, cell };
}


template <typename T>
auto Async::Task<T>::block() -> Async::Result<T> {
    auto cell_result = this->cell->block();
    return Cell::map_result(cell_result, 
        [](T value) { return { value }; },
        [](Async::Error err) { return { err }; });
}


template <typename T>
auto Async::Task<T>::task_list_to_cell_list(std::vector<Task<T>> tasks) -> std::vector<std::shared_ptr<Cell::ICell<T, Async::Error>>> {
    auto cells = std::vector<std::shared_ptr<Cell::ICell<T, Async::Error>>>();
    for (auto& task : tasks) {
        cells.push_back(task.cell);
    }

    return cells;
}

template <typename T>
auto Async::Task<T>::when_any(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<T> {
    auto cells = task_list_to_cell_list(tasks);
    auto when_any_cell = std::make_shared<Cell::WhenAnyCell<T, Async::Error>>(scheduler, cells);
    return { scheduler, when_any_cell };
}


template <typename T>
auto Async::Task<T>::when_all(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<std::vector<T>> {
    auto cells = task_list_to_cell_list(tasks);
    auto when_all_cell = std::make_shared<Cell::WhenAllCell<T, Async::Error>>(scheduler, cells);
    return { scheduler, when_all_cell };
}