#include "async_lib/task_timer_source.h"

auto Async::TaskTimerSource::after(std::chrono::milliseconds duration) -> Async::Task<Unit> {
    auto value_source = std::make_shared<Async::TaskValueSource<Unit>>(scheduler);
    // the value source triggers after the expiry, this is achieved by
    // scheduling a task to complete the value source after the expiry
    timing_poll_source.get().schedule(duration, [value_source](auto ctx) {
        value_source->complete(ctx, {}); 
    });

    return value_source->create();
}