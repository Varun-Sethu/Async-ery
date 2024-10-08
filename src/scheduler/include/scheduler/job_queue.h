#pragma once

#include <functional>
#include <optional>
#include <mutex>
#include <vector>

#include "concurrency/spinlock.h"
#include "scheduler/job.h"

class JobQueue {
public:
    explicit JobQueue(size_t base_size) : queue(base_size) {}
    JobQueue() : JobQueue(default_size) {}
    ~JobQueue() = default;
        
    JobQueue(JobQueue& other) = delete;
    JobQueue(JobQueue&& other) noexcept: 
        queue(std::move(other.queue)),
        head(other.head),
        tail(other.tail),
        current_size(other.current_size.load()) {}

    auto operator=(JobQueue& other) -> JobQueue& = delete;
    auto operator=(JobQueue&& other) noexcept -> JobQueue& {
        queue = std::move(other.queue);
        head = other.head;
        tail = other.tail;
        current_size.store(other.current_size.load());
        return *this;
    }

    auto enqueue(Scheduler::Job&& item) -> void;
    auto dequeue() -> std::optional<Scheduler::Job>;

    // Note: size has a relaxed memory order, hence even tho size may return a non-zero value
    // there is no guarantee that the next time you attempt to dequeue, the value will be non-null
    auto size() const -> size_t;

private:
    // Resize grows the queue to be a factor of two multiplier of the current size
    // NOTE: the function assumes the lock over the queue is currently held
    auto resize() -> void;


    std::vector<Scheduler::Job> queue;
    size_t head = 0;
    size_t tail = 0;

    // Prevent the spin lock and the current size from being within the same cache line to prevent
    // false sharing
    // TODO: make this not a constant specific to x86-64, there exists std::hardware_destructive_interference_size
    const static size_t cache_line_size = 64;
    const static size_t default_size = 1024;

    alignas(cache_line_size) mutable SpinLock spinlock;
    alignas(cache_line_size) mutable std::atomic<size_t> current_size = { 0 };
};