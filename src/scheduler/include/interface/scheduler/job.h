#pragma once

#include <functional>

#include "scheduler/scheduling_context.h"

namespace Scheduler {
    using Job = std::function<void(Context)>;
}
