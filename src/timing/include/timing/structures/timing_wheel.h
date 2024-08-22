#pragma once

#include <chrono>
#include <vector>
#include <ranges>

// TimingWheel implements a timing wheel with a defined resolution (tick size)
//  - To schedule the execution of some timer in the future invoke the schedule() fn with the
//    amount of time in the future you want to schedule the timer and the timer itself
//
//  - To advance the wheel invoke the advance() fn, this will return all the timers that expired
//    during that advancement

template <typename Timer>
class TimingWheel {
    using TimeBucket = std::vector<Timer>;

    public:
        TimingWheel(std::chrono::milliseconds wheel_tick_size, size_t num_ticks);

        auto schedule(std::chrono::milliseconds duration_from_last_advancement, Timer&& timer) -> void;
        auto advance() -> std::vector<Timer>;

    private:
        auto inline non_wrapped_wheel_index(std::chrono::system_clock::time_point time) -> size_t;

        std::vector<TimeBucket> wheel;
        std::chrono::milliseconds wheel_tick_size;
        std::chrono::system_clock::time_point last_advancement_time;

        size_t num_ticks;
        size_t current_wheel_index = 0;        
};







template <typename Timer>
TimingWheel<Timer>::TimingWheel(std::chrono::milliseconds wheel_tick_size, size_t num_ticks) :
    wheel(std::vector<TimeBucket>(num_ticks)),
    wheel_tick_size(wheel_tick_size),
    last_advancement_time(std::chrono::system_clock::now()),
    num_ticks(num_ticks)
{
    for (auto& bucket : wheel) {
        bucket = TimeBucket();
    }
}

// non_wrapped_wheel_index calculates the index of the wheel that a given time would fall into
// note that it doesn't normalize the index by taking the modulus against the wheel size, hence
// the nameL non_wrapped_wheel_index
template <typename Timer>
auto inline TimingWheel<Timer>::non_wrapped_wheel_index(std::chrono::system_clock::time_point time) -> size_t {
    return current_wheel_index + static_cast<size_t>((time - last_advancement_time) / wheel_tick_size);
}

template <typename Timer>
auto TimingWheel<Timer>::schedule(std::chrono::milliseconds duration_from_last_advancement, Timer&& timer) -> void {
    auto index = non_wrapped_wheel_index(last_advancement_time + duration_from_last_advancement);
    auto time_bucket = index % num_ticks;
    wheel[time_bucket].push_back(std::move(timer));
}

template <typename Timer>
auto TimingWheel<Timer>::advance() -> std::vector<Timer> {
    auto now = std::chrono::system_clock::now();
    if (now - last_advancement_time < wheel_tick_size) { return std::vector<Timer>(); }

    // we only normalize to a concrete index within the loop body
    // this ensures we automatically deal with the fact that we have have wrapped around
    // the wheel several times during this current_time_index
    auto resolved_timers = std::vector<Timer>();
    auto completed_buckets = std::views::iota(current_wheel_index, non_wrapped_wheel_index(now))
                                | std::views::transform([this](auto idx) { return idx % num_ticks; });

    // iterate over the wheel and collect all the timers
    for (auto idx : completed_buckets) {
        for (auto& entry : wheel[idx]) {
            resolved_timers.push_back(std::move(entry));
        }

        // erase the wheel bucket
        wheel[idx].clear();
    }
            
    current_wheel_index = non_wrapped_wheel_index(now);
    last_advancement_time = now;

    return resolved_timers;
}