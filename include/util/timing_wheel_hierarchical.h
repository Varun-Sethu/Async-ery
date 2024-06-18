#pragma once

#include <chrono>
#include <vector>
#include <ranges>


template <typename Timer>
class HierarchicalTimingWheel {
    public:
        HierarchicalTimingWheel(std::chrono::milliseconds tick_size, std::vector<size_t> wheel_sizes);
        auto schedule(std::chrono::milliseconds duration_from_last_advancement, Timer&& timer) -> void;
        auto advance() -> std::vector<Timer>;
    private:
        auto load_timers_from_wheel(size_t wheel_num) -> void;
        auto determine_timer_wheel(size_t ticks_since_last_advancement) -> std::tuple<size_t, size_t>;
        auto inline determine_new_bottom_wheel_index(std::chrono::system_clock::time_point now) -> size_t;

        // TimerEntry contains a timer + some tick_offset_into_bucket
        // an tick_offset_into_bucket represents the amount of "extra" ticks a timer is scheduled for in a bucket
        // ie. consider the buckets [0, 200), [200, 400) with a tick size of 100ms.
        // a timer scheduled for 300ms will be in the 2nd bucket, since there is an extra 100ms left 
        // after the bucket for the timer, timer has an tick_offset_into_bucket of 100ms or 1 tick. As timers move between
        // hierarchies their offsets change, offsets purely exist for book-keeping purposes to determine
        // where in the lower heirarchy to place a timer.
        struct TimerEntry {
            size_t tick_offset_into_bucket;
            Timer timer;
        };

        // Wheel models an individual wheel within the hierarchical timing wheel.
        // Each wheel consists of a specfied number of buckets that wheel can hold, the number of ticks
        // held in each bucket as well as the current index that the wheel is at. We coupled this data together
        // for easier book-keeping.
        struct Wheel {
            size_t num_buckets;
            size_t ticks_per_bucket;

            size_t curr_bucket_index;
            std::vector<std::vector<TimerEntry>> buckets;
        };

        std::chrono::milliseconds tick_size;
        std::chrono::system_clock::time_point last_advancement_time;
        std::vector<Wheel> wheels;
};



template <typename Timer>
HierarchicalTimingWheel<Timer>::HierarchicalTimingWheel(std::chrono::milliseconds tick_size, std::vector<size_t> wheel_sizes) :
    tick_size(tick_size),
    last_advancement_time(std::chrono::system_clock::now())
{
    /**
     * HierarchicalTimingWheels are structured such that every wheel can be fully contained within a BUCKET of the wheel above it. 
     * ie. a possible setup is an inner wheel that represents 1 whole minute and an outer wheel that represents an hour, with 60 buckets.
     * 
     * This leads to the natural fact that every wheel in this hierarchy can store a varying amount of ticks.  
     */
    auto total_ticks_in_last_wheel = size_t(1);
    for (auto& wheel_size : wheel_sizes) {
        wheels.push_back(Wheel {
            .num_buckets = wheel_size,
            .ticks_per_bucket = total_ticks_in_last_wheel,
            .curr_bucket_index = 0,
            .buckets = std::vector<std::vector<TimerEntry>>(wheel_size)
        });

        total_ticks_in_last_wheel *= wheel_size;
    }
}





template <typename Timer>
auto HierarchicalTimingWheel<Timer>::schedule(std::chrono::milliseconds duration_from_last_advancement, Timer&& timer) -> void {
    auto ticks_to_fit = duration_from_last_advancement / tick_size;

    auto [wheel_to_place_in, ticks_left] = determine_timer_wheel(ticks_to_fit);
    auto& [num_buckets, ticks_per_bucket, curr_bucket_index, buckets] = wheels[wheel_to_place_in];

    auto timer_bucket_index = (curr_bucket_index + (ticks_left / ticks_per_bucket)) % num_buckets;
    auto tick_offset_into_bucket = ticks_left % ticks_per_bucket;
    buckets[timer_bucket_index].push_back(TimerEntry { tick_offset_into_bucket, std::move(timer) });
}


template <typename Timer>
auto HierarchicalTimingWheel<Timer>::advance() -> std::vector<Timer> {
auto now = std::chrono::system_clock::now();
    if (now - last_advancement_time < tick_size) { return std::vector<Timer>(); }
            
    auto resolved_timers = std::vector<Timer>();
    auto& [lowest_wheel_size, _, lowest_wheel_bucket_index, lowest_wheel] = wheels[0];
    auto completed_buckets = std::views::iota(lowest_wheel_bucket_index, determine_new_bottom_wheel_index(now))
                           | std::views::transform([lowest_wheel_size](auto i) { return i % lowest_wheel_size; });

    // keep reading all the timers from each bucket until we reach the current time
    for (auto i : completed_buckets) {
        for (auto& [_, timer] : lowest_wheel[i]) { resolved_timers.push_back(std::move(timer)); }

        // clear this bucket and advance the current bucket index
        // to the next bucket, if we've wrapped around to 0 we need to load all events from the wheel above us
        lowest_wheel[i].clear();
        lowest_wheel_bucket_index = (lowest_wheel_bucket_index + 1) % lowest_wheel_size;
        if (lowest_wheel_bucket_index == 0) { load_timers_from_wheel(/* wheel_num = */ 1); }
    }

    last_advancement_time = now;
    return resolved_timers;
}


// load_timers_from_wheel will load all the timers from the wheel at wheel_num into the wheel at wheel_num - 1
// this is done to ensure that we don't have to iterate through all the wheels to find the timers that need to be executed.
// During each load it recomputes the "offset" of that timer into this wheel -> that is how many ticks into the bucket the timer is.
// For more on offsets see the documentation under TimerEntry.
template <typename Timer>
auto HierarchicalTimingWheel<Timer>::load_timers_from_wheel(size_t wheel_num) -> void {
    if (wheel_num == wheels.size() || wheel_num == 0) { return; }

    auto& [num_buckets, _, wheel_index, wheel] = wheels[wheel_num];
    auto& [num_buckets_below, ticks_per_bucket_below, wheel_index_below, wheel_below] = wheels[wheel_num - 1];

    // populate the wheel below wheel_num with the contents of the current wheel_num index
    for (auto& [tick_offset_into_bucket, timer] : wheel[wheel_index]) {
        auto bucket_index = (wheel_index_below + (tick_offset_into_bucket / ticks_per_bucket_below)) % num_buckets_below;
        auto new_tick_offset_into_bucket = tick_offset_into_bucket - (bucket_index * ticks_per_bucket_below);
        wheel_below[bucket_index].push_back(TimerEntry { new_tick_offset_into_bucket, std::move(timer) });
    }

    // clear the entry for this wheel's moved bucket
    wheel[wheel_index].clear();            
    wheel_index = (wheel_index + 1) % num_buckets;
    if (wheel_index == 0) { load_timers_from_wheel(wheel_num + 1); }
}


// determine_timer_wheel will determine the wheel that a specific "duration" falls in.
// The logic for finding this heirarchy is somewhat involved.
//  In general we need to identify what heirarchy to place the duration in, we do so by first determining the amount of ticks into the 
//  future is required for storing this duration. Each heirarchy can support a certain amount of ticks into the future. Since for each member in the
//  heirarchy we can store the entire below it in a single bucket each level supports a different amount of ticks, for the example below assume
//  that the level 1 heirarchy supports 2 ticks per bucket
//
//    lvl.3 [2 buckets] -> can hold an EXTRA 12 * 2 = 24 ticks -> holds total of 40 ticks
//    lvl.2 [3 buckets] -> can hold an EXTRA 4 * 3 = 12 ticks  -> holds total of 16 ticks
//    lvl.1 [2 buckets] -> can hold 4 ticks                    -> holds total of 4 ticks
//
//  We basically just keep walking up this heirarchy until we find a bucket that can hold the duration, if we reach the top we just place it in the top
//  bucket. Once we've found a heirarchy to place the duration in we calculate the index of the bucket in the heirarchy to place it in. Another thing to
//  note is that it's technically not as simple as "can this bucket hold the duration" since we also need to consider what the current index into the bucket is
//  if the bucket's index is at 2 and the bucket for this timer is 0, we need to move the timer to the next heirarchy.
//  Alongside this, as we progress up the heirarchy we need to subtract the ticks from the previous heirarchy since they are consumed by the heirarchy below.
template <typename Timer>
auto HierarchicalTimingWheel<Timer>::determine_timer_wheel(size_t ticks_since_last_advancement) -> std::tuple<size_t, size_t> {
    auto ticks_to_fit = ticks_since_last_advancement;
    auto can_fit_in_wheel = [&wheels=this->wheels, &ticks_to_fit](auto wheel) {
        if (wheel == wheels.size() - 1) { return true; }

        auto& [num_buckets, ticks_per_bucket, wheel_index, _] = wheels[wheel];
        return wheel_index + (ticks_to_fit / ticks_per_bucket) < num_buckets;
    };

    auto curr_wheel = size_t(0);
    while (!can_fit_in_wheel(curr_wheel)) {
        ticks_to_fit -= wheels[curr_wheel].num_buckets * wheels[curr_wheel].ticks_per_bucket;
        curr_wheel += 1;
    }

    return { curr_wheel, ticks_to_fit };
}


template <typename Timer>
auto inline HierarchicalTimingWheel<Timer>::determine_new_bottom_wheel_index(std::chrono::system_clock::time_point now) -> size_t {
    auto& [_, __, lowest_wheel_bucket_index, ___] = wheels[0];
    auto bucket_index = lowest_wheel_bucket_index + ((now - last_advancement_time) / tick_size);
    return bucket_index;
}