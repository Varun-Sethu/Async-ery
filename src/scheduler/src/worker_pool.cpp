#include <cassert>
#include <utility>
#include <vector>
#include <optional>
#include <random>
#include <cstddef>

#include "scheduler/worker_pool.h"
#include "scheduler/worker.h"
#include "scheduler/job.h"
#include "scheduler/scheduling_context.h"

Scheduler::WorkerPool::WorkerPool(unsigned int n_workers) {
    for (unsigned int i = 0; i < n_workers; i++) {
        workers.emplace_back(
            Context(i), 
            [this]() { return this->find_new_work(); });
    }

    // start all the workers
    for (auto& worker : workers) {
        auto could_start = worker.start();
        assert(could_start && "Failed to start worker");
    }
}

auto Scheduler::WorkerPool::queue(Context ctx, Job&& job) -> void {
    auto worker_id = ctx.worker_id;
    if (worker_id.has_value()) {
        auto& worker = workers[worker_id.value()];
        worker.queue(std::move(job));
    } else {
        global_queue.enqueue(std::move(job));
    }
}

// TODO: Change work allocation policy such that work is allocated to workers instead of a global queue
//       this will allow for better load balancing and less contention over the global queue
auto Scheduler::WorkerPool::find_new_work() -> std::optional<Job> {
    // check the global queue for any jobs
    if (auto job = global_queue.dequeue(); job.has_value()) {
        return job;
    }

    // we must now evict a job from another worker queue, we do this by from some random worker
    // until we wrap around, we do this "approximately" skipping if we see no jobs
    auto ran_dev = std::random_device();
    auto mersene = std::mt19937(ran_dev());
    auto dist = std::uniform_int_distribution<int>(0, static_cast<int>(workers.size()) - 1);

    auto num_workers = workers.size();
    auto random_worker = static_cast<size_t>(dist(mersene));

    for (size_t i = 0; i < num_workers; i++) {
        auto worker_id = (random_worker + i) % num_workers;
        if (auto job = workers[worker_id].steal_job(); job.has_value()) {
            return job;
        }
    }

    return {};
}