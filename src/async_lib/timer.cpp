#include "async_lib/timer.h"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

#include "interface/timing/timer_poll_source_intf.h"

Timer::Timer(Duration duration, Callback callback, Timing::IPollSource& timing_source)
    : state_(std::make_shared<State>())
{
    state_->user_callback = std::move(callback);
    state_->duration = duration;
    state_->timing_source = &timing_source;
}

auto Timer::reprogram() -> void {
    const std::lock_guard lock(state_->mutex);

    if (state_->timer_is_running) {
        state_->timer_was_reprogrammed = true;
    } else {
        state_->timer_is_running = true;
        const std::weak_ptr<State> weak_state = state_;
        state_->timing_source->schedule(state_->duration, [weak_state](auto) {
            on_timing_source_callback(weak_state);
        });
    }
}

auto Timer::on_timing_source_callback(const std::weak_ptr<State>& weak_state) -> void {
    auto state = weak_state.lock();
    if (!state) {
        return;
    }

    std::unique_lock lock(state->mutex);

    if (state->timer_was_reprogrammed) {
        state->timer_was_reprogrammed = false;
        lock.unlock();
        state->timing_source->schedule(state->duration, [weak_state](auto) {
            on_timing_source_callback(weak_state);
        });
        return;
    }

    state->timer_is_running = false;
    auto callback = state->user_callback;
    lock.unlock();
    callback();
}
