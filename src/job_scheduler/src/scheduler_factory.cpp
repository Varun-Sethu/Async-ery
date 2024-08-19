#include "job_scheduler/scheduler_factory.h"
#include "job_scheduler/job_scheduler.h"

auto Scheduler::create_scheduler(int n_workers, std::vector<std::shared_ptr<IPollSource>> poll_sources) -> std::unique_ptr<IJobScheduler> {
    auto scheduler = std::make_unique<JobScheduler>(n_workers, poll_sources);
    auto i_scheduler = std::unique_ptr<IJobScheduler>(std::move(scheduler));
    return i_scheduler;
}