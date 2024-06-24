#include <iostream>

#include "util/job_queue.h"


auto JobQueue::enqueue(SchedulerTypes::Job&& job) -> void {
    std::lock_guard<SpinLock> lock(spinlock);

    auto is_full = (tail + 1) % queue.size() == head;
    if (is_full) { resize(); }

    queue[tail] = std::move(job);
    tail = (tail + 1) % queue.size();   
}

auto JobQueue::dequeue() -> std::optional<SchedulerTypes::Job> {
    std::lock_guard<SpinLock> lock(spinlock);
    if (head == tail) { return std::nullopt; }

    auto job = std::optional(std::move(queue[head]));
    head = (head + 1) % queue.size();

    return job;
}

auto JobQueue::size() -> size_t {
    std::lock_guard<SpinLock> lock(spinlock);
    return tail >= head
                ? tail - head
                : queue.size() - head + tail; 
}

// Resize grows the queue to be a factor of two multiplier of the current size
// NOTE: the function assumes the lock over the queue is currently held
auto JobQueue::resize() -> void {
    auto old_size = queue.size();
    auto new_size = queue.size() * 2;
    queue.resize(new_size);

    // we must now reposition all elements in the queue to their correct positions
    // in the resized queue
    auto new_tail = head;
    for (auto entry = head; (entry % old_size) != tail; entry += 1) {
        if (entry % old_size != entry % new_size) {
            queue[entry % new_size] = std::move(queue[entry % old_size]);
        }

        new_tail += 1;
    }

    tail = new_tail % new_size;
}