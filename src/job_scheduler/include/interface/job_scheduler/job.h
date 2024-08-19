#pragma once

#include <functional>

#include "job_scheduler/scheduling_context.h"

namespace Scheduler {
    using Job = std::function<void(Context)>;
}
