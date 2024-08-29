// NOLINTBEGIN

#include <vector>
#include <iostream>
#include <variant>
#include <cmath>
#include <numeric>
#include <iterator>

#include "async_lib/task_factory.h"


template<typename It>
auto sum_range(Async::TaskFactory& factory, It start, It end) -> Async::Task<int> {
    return factory.create<int>([start, end]() {
        auto sum = 0;
        for (auto it = start; it != end; it = std::next(it)) {
            sum += *it;
        }

        return sum;
    });
}


auto concurrent_sum(Async::TaskFactory& factory, const size_t max_values_per_task, std::vector<int>&& values) -> Async::Task<int> {
    auto tasks = std::vector<Async::Task<int>> {};

    for (auto i = size_t(0); i < values.size(); i += max_values_per_task) {
        auto range_start = std::next(values.begin(), static_cast<long>(i));
        auto range_end = i + max_values_per_task >= values.size() 
                            ? values.end() 
                            : std::next(values.begin(), static_cast<long>(i + max_values_per_task)) ;

        tasks.push_back(sum_range(factory, range_start, range_end));
    };

    auto sum_vec = [](auto sums) { return std::accumulate(sums.begin(), sums.end(), 0); };
    return factory.when_all(tasks)
                  .map<int>(sum_vec);
}



// Example demonstrating the use of the TaskFactory class to concurrently sum a range of numbers
// from 1 to n using 10 worker threads.
auto main() -> int {
    auto factory = Async::TaskFactory(/* N_WORKERS = */ 10);
    
    auto n = 100;
    auto max_values_per_task = size_t(10);
    std::cout << "Enter a number n you wish to sum up to: \n";
    std::cin >> n;

    std::cout << "Enter the maximum number of values per task: \n";
    std::cin >> max_values_per_task;
    
    auto summation_input = std::vector<int>(static_cast<size_t>(n));
    std::iota(summation_input.begin(), summation_input.end(), 1);
    
    auto summation_task = concurrent_sum(factory, max_values_per_task, std::move(summation_input));
    std::cout << "Summation of 1 to " << n << ": " << std::get<int>(summation_task.block()) << std::endl;
}


// NOLINTEND