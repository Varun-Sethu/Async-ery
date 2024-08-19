#include "scheduler/scheduler_factory.h"
#include "scheduler/scheduler.h"

auto Scheduler::create_scheduler(int n_workers, std::vector<std::shared_ptr<IPollSource>> poll_sources) -> std::unique_ptr<IScheduler> {
    auto scheduler = std::make_unique<Scheduler>(n_workers, poll_sources);
    auto i_scheduler = std::unique_ptr<IScheduler>(std::move(scheduler));
    return i_scheduler;
}