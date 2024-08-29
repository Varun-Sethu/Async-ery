// NOLINTBEGIN

#include <iostream>
#include <vector>
#include <chrono>

#include "async_lib/task_factory.h"


auto main() -> int {
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
    auto task_source_one = task_factory.value_source<int>();
    auto task_source_two = task_factory.value_source<int>();

    auto when_any_task = task_factory.when_any<int>({
        task_source_one.create().map<int>([](int x) { return x + 5; }),
        task_source_one.create().map<int>([](int x) { return 4 * x + 5; }),
        task_source_two.create().map<int>([](int x) { return 3 * x + 5; }),
    });

    auto when_all_task = task_factory.when_all<int>({
        task_source_one.create().map<int>([](int x) { return x + 5; }),
        task_source_one.create().map<int>([](int x) { return 4 * x + 5; }),
        task_source_two.create().map<int>([](int x) { return 3 * x + 5; }),
    });

    task_source_one.complete(100);
    task_source_two.error(Async::Error::Rejected);

    std::cout << "Waiting for computation to complete..." << std::endl;
    std::cout << "Result when any: " << std::get<int>(when_any_task.block()) << std::endl;
    std::cout << "Result when all: " << Async::error_to_string(std::get<Async::Error>(when_all_task.block())) << std::endl;
}

// NOLINTEND