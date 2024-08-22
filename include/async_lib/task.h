#pragma once

#include <iostream>
#include <functional>
#include <memory>
#include <ranges>

#include "types.h"
#include "scheduler/scheduler_intf.h"
#include "cell/write_once_cell.h"
#include "cell/tracking_once_cell.h"
#include "cell/when_any_cell.h"
#include "cell/when_all_cell.h"

namespace Async {
    template <typename T>
    class TaskValueSource;  // see comment for TaskValueSource in task_value_source.h

    // Task is a class that represents a task that can be awaited
    // it is simply just a wrapper around a IReadableCell and prevents direct writes to the cell
    // Abstracting over direct writes to a cell allows multiple tasks to be driven by the same underlying
    // cell SAFELY
    template <typename T>
    class Task {
        template <typename Q> friend class Task;
        friend class TaskValueSource<T>;   // for exposing private Task constructor that takes a cell
        
        public:
            Task(Scheduler::IScheduler& scheduler, std::function<T(void)> func);

            
            template <typename G>
            auto bind(std::function<Task<G>(T)> func) -> Task<G>;


            template <typename G>
            auto map(std::function<G(T)> func) -> Task<G>;

            // block will pause the current thread until the value of the cell is available
            // it will then return the value of the cell.
            auto block() -> T;

            // when_any is a task that resolves when any of the provided underlying tasks are resolved
            // under the hood the task claims shared ownership of the cells it is tracking
            static auto when_any(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<T>;

            // when_all resolves when all the underlying tasks have resolved, returning the values of the cells
            // in a final vector whose order matches the order of the tasks
            static auto when_all(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<std::vector<T>>;

        protected:
            // ICell are an implementation detail so creation of Tasks from them is restricted
            // to be exclusively a private constructor
            Task(Scheduler::IScheduler& scheduler, std::shared_ptr<Cell::ICell<T>> cell) : 
                cell(std::move(cell)), scheduler(scheduler) {}
        
        private:
            std::shared_ptr<Cell::ICell<T>> cell;
            //  Note: it is an invariant of the Asynchronous library that the scheduler's
            //        lifetime is longer than the lifetime of any task / cell that uses it.
            //        in the application scope it has a 'static lifetime
            std::reference_wrapper<Scheduler::IScheduler> scheduler;
    };
}






// Implementation
template <typename T>
Async::Task<T>::Task(Scheduler::IScheduler& scheduler, std::function<T(void)> func) : scheduler(scheduler) {
    auto cell = std::make_shared<Cell::WriteOnceCell<T>>(scheduler);
    this->cell = cell;
    this->scheduler.get().queue(Scheduler::Context::empty(), [cell, func](auto ctx) {
        auto result = func();
        cell->write(ctx, result);
    });
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
    auto tracking_cell = std::make_shared<Cell::TrackingOnceCell<G>>();
    this->cell->await([tracking_cell, func](__attribute__((unused)) auto ctx, T value) {
        auto new_task = func(value);
        tracking_cell->track(new_task.cell);
    });

    return Task<G>(scheduler, tracking_cell);
}


// map can be implemented rather simply as a invocation of bind
// however for efficiency reasons we do not do this and instead 
// implement map using a WORM cell
template <typename T>
template <typename G>
auto Async::Task<T>::map(std::function<G(T)> func) -> Task<G> {
    auto cell = std::make_shared<Cell::WriteOnceCell<G>>(scheduler);
    this->cell->await([cell, func](auto ctx, T value) {
        auto result = func(value);
        cell->write(ctx, result);
    });

    return Task<G>(scheduler, cell);
}


template <typename T>
auto Async::Task<T>::block() -> T { return this->cell->block(); }


template <typename T>
auto Async::Task<T>::when_any(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<T> {
    auto cell_view = tasks | std::views::transform([](auto& task) { return task.cell; });
    auto cell_list = std::vector<std::shared_ptr<Cell::ICell<T>>>(cell_view.begin(), cell_view.end()); 

    return Task<T>(scheduler, std::make_shared<Cell::WhenAnyCell<T>>(scheduler, cell_list));
}


template <typename T>
auto Async::Task<T>::when_all(Scheduler::IScheduler& scheduler, std::vector<Task<T>> tasks) -> Task<std::vector<T>> {
    auto cell_view = tasks | std::views::transform([](auto& task) { return task.cell; });
    auto cell_list = std::vector<std::shared_ptr<Cell::ICell<T>>>(cell_view.begin(), cell_view.end());

    return Task<std::vector<T>>(scheduler, std::make_shared<Cell::WhenAllCell<T>>(scheduler, cell_list));
}