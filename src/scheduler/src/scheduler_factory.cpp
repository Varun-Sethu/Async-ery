#include <memory>
#include <vector>
#include <utility>

#include "scheduler/scheduler_factory.h"
#include "scheduler/scheduler.h"
#include "scheduler/poll_source.h"
#include "scheduler/scheduler_intf.h"

auto Scheduler::create_scheduler(int n_workers, const std::vector<std::shared_ptr<IPollSource>>& poll_sources) -> std::unique_ptr<IScheduler> {
    auto scheduler = std::make_unique<Scheduler>(n_workers, poll_sources);
    auto i_scheduler = std::unique_ptr<IScheduler>(std::move(scheduler));
    return i_scheduler;
}