#pragma once

#include <functional>
#include <optional>
#include <mutex>
#include <vector>

#include "util/spinlock.h"
#include "scheduler/types.h"


class CircularQueue {
    public:
        CircularQueue(int base_size) : queue(base_size) {}

        auto enqueue(Async::SchedulerJob&& item) -> void;
        auto dequeue() -> std::optional<Async::SchedulerJob>;
        auto size() -> size_t;

    private:
        // Resize grows the queue to be a factor of two multiplier of the current size
        // NOTE: the function assumes the lock over the queue is currently held
        auto resize() -> void;


        std::vector<Async::SchedulerJob> queue;
        size_t head = 0;
        size_t tail = 0;

        SpinLock spinlock;
};