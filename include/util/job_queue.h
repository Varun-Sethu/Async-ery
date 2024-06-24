#pragma once

#include <functional>
#include <optional>
#include <mutex>
#include <vector>

#include "spinlock.h"
#include "types.h"


class JobQueue {
    public:
        JobQueue(int base_size) : queue(base_size) {}

        auto enqueue(SchedulerTypes::Job&& job) -> void;
        auto dequeue() -> std::optional<SchedulerTypes::Job>;
        auto size() -> size_t;

    private:
        // Resize grows the queue to be a factor of two multiplier of the current size
        // NOTE: the function assumes the lock over the queue is currently held
        auto resize() -> void;


        std::vector<SchedulerTypes::Job> queue;
        size_t head = 0;
        size_t tail = 0;

        SpinLock spinlock;
};