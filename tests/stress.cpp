#include <iostream>
#include <variant>
#include <thread>
#include <chrono>

#include "async_lib/task.h"
#include "async_lib/task_timer_source.h"
#include "scheduler/scheduler.h"

using std::chrono_literals::operator""ms;


auto main() -> int {
    // Async runtime setup
    auto timing_poll_source = std::make_shared<Async::TimingPollSource>();
    auto scheduler = Async::Scheduler(/* N_WORKERS = */ 3, { timing_poll_source });
    auto timing_source = Async::TaskTimerSource(scheduler, *timing_poll_source);
    
    auto counter = std::atomic<int>(0);

    for (int j = 0; j < 1000; j++) {
        auto offset = j * 100ms;
        timing_source.after(1000ms + offset).map<int>([j, &counter](auto _) {
            counter.fetch_add(1);
            return 0;
        });
    }

    while (counter.load() < 1000) { std::this_thread::yield(); }
    std::cout << "All done!" << std::endl;

    return 0;
}