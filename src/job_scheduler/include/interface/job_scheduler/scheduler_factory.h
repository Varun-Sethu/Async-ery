#pragma once

#include <initializer_list>
#include <memory>

#include "job_scheduler/job_scheduler_intf.h"
#include "job_scheduler/poll_source.h"


namespace Scheduler {
    auto create_scheduler(int n_workers, std::vector<std::shared_ptr<IPollSource>> poll_sources) -> std::unique_ptr<IJobScheduler>;
}