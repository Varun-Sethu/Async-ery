// NOLINTBEGIN
//  Note: this is just a test file to demonstrate how to use the library
//        and is not a part of the library itself.

#include <iostream>
#include <vector>
#include <chrono>

#include "async_lib/task_factory.h"

using std::chrono_literals::operator""ms;

auto delay_by(Async::TaskTimerSource& timer_source, std::chrono::milliseconds amount) -> std::function<Async::Task<int>(int)> {
    return [&timer_source, amount](int value) -> Async::Task<int> {
        return timer_source
                .after(amount)
                .map<int>([value](__attribute__((unused)) auto _) -> int { return value; });
    };
}

// models a computation that resolves when any of the tasks resolve
auto when_any_computation(Async::TaskFactory& task_factory, Async::TaskValueSource<int>& task_source, Async::TaskTimerSource& timer_source) -> Async::Task<int> {
    return task_factory.when_any<int>({
        task_source.create().bind<int>(delay_by(timer_source, 300ms)).map<int>([](int x) { return x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 500ms)).map<int>([](int x) { return 4 * x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 400ms)).map<int>([](int x) { return 3 * x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 200ms)).map<int>([](int x) { return 2 * x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 1500ms)).map<int>([](int x) { return 6 * x + 5; }),
        task_source.create()
            .map<int>([](int x) { return x + 5; })
            .map<int>([](int x) { return 2 * x + 5; })
            .map<int>([](int x) { return x * 2; })
            .bind<int>(delay_by(timer_source, 300ms)),
    });
}

// models a computation that resolves when all of the tasks resolve
auto when_all_computation(Async::TaskFactory& task_factory, Async::TaskValueSource<int>& task_source, Async::TaskTimerSource& timer_source) -> Async::Task<std::vector<int>> {
    return task_factory.when_all<int>({
        task_source.create().bind<int>(delay_by(timer_source, 300ms)).map<int>([](int x) { return x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 500ms)).map<int>([](int x) { return 4 * x + 5; }),
        task_source.create().bind<int>(delay_by(timer_source, 400ms)).map<int>([](int x) { return 3 * x + 5; }),
    });
}


auto main() -> int {
    // Async runtime setup
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
    auto task_source = task_factory.value_source<int>();
    auto timing_source = task_factory.timer_source();

    // creating tasks! :D - note that we're simulating a delay here
    auto when_any = when_any_computation(task_factory, task_source, timing_source);
    auto when_all = when_all_computation(task_factory, task_source, timing_source);
    task_source.complete(100);

    std::cout << "Waiting for computation to complete..." << std::endl;
    std::cout << "Result when any: " << when_any.block() << std::endl;
    std::cout << "Result when all: ";
    for (auto& x : when_all.block()) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

// NOLINTEND