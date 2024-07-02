#pragma once

#include <functional>
#include <optional>
#include <mutex>
#include <vector>
#include <new>

#include "util/spinlock.h"
#include "scheduler/types.h"


class CircularQueue {
    public:
        CircularQueue(int base_size) : queue(base_size) {}
        
        ~CircularQueue() = default;
        CircularQueue(CircularQueue&& other) : queue(std::move(other.queue)), head(other.head), tail(other.tail) {}
        CircularQueue(CircularQueue& other) = delete;
        auto operator=(CircularQueue& other) -> CircularQueue& = delete;
        auto operator=(CircularQueue&& other) -> CircularQueue& {
            queue = std::move(other.queue);
            head = other.head;
            tail = other.tail;
            return *this;
        }

        auto enqueue(Async::SchedulerJob&& item) -> void;
        auto dequeue() -> std::optional<Async::SchedulerJob>;

        // Note: size has a relaxed memory order, hence even tho size may return a non-zero value
        // there is no guarantee that the next time you attempt to dequeue, the value will be non-null
        auto size() -> size_t;

    private:
        // Resize grows the queue to be a factor of two multiplier of the current size
        // NOTE: the function assumes the lock over the queue is currently held
        auto resize() -> void;


        std::vector<Async::SchedulerJob> queue;
        size_t head = 0;
        size_t tail = 0;

        // Prevent the spin lock and the current size from being within the same cache line to prevent
        // false sharing
        // TODO: make this not a constant specific to x86-64, there exists std::hardware_destructive_interference_size
        alignas(64) SpinLock spinlock;
        alignas(64) std::atomic<size_t> current_size = { 0 };
};