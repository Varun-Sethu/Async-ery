#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>

#include "interface/timing/timer_poll_source_intf.h"

class Timer {
public:
    using Callback = std::function<void()>;
    using Duration = std::chrono::milliseconds;

    Timer(Duration duration, Callback callback, Timing::IPollSource& timing_source);

    auto reprogram() -> void;

private:
    struct State {
        std::mutex mutex;
        bool timer_was_reprogrammed = false;
        bool timer_is_running = false;
        Callback user_callback;
        Duration duration;
        Timing::IPollSource* timing_source;
    };

    std::shared_ptr<State> state_;

    static auto on_timing_source_callback(const std::weak_ptr<State>& weak_state) -> void;
};
