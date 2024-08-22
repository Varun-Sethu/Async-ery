#include <assert.h>

#include "scheduler/worker_pool.h"

Scheduler::WorkerPool::WorkerPool(int n_workers) {
    for (int i = 0; i < n_workers; i++) {
        auto worker = JobWorker(Context(i), [this]() { return this->find_new_work(); });
        workers.emplace_back(std::move(worker));
    }

    // start all the workers
    for (auto& worker : workers) {
        assert(worker.start() && "Failed to start worker");
    }
}

auto Scheduler::WorkerPool::queue(Context ctx, Job job) -> void { queue(ctx, std::vector<Job> { std::move(job) }); }
auto Scheduler::WorkerPool::queue(Context ctx, std::vector<Job> jobs) -> void {
    if (ctx != Context::empty()) {
        auto& worker = workers[ctx.worker_id.value()];
        worker.queue(std::move(jobs));
    } else {
        for (auto& job : jobs) {
            global_queue.enqueue(std::move(job));
        }
    }
}


auto Scheduler::WorkerPool::find_new_work() -> std::optional<Job> {
    // check the global queue for any jobs
    if (auto job = global_queue.dequeue(); job.has_value()) {
        return job;
    }

    // we must now evict a job from another worker queue, we do this by from some random worker
    // until we wrap around, we do this "approximately" skipping if we see no jobs
    srand(static_cast<unsigned int>(time(nullptr)));
    auto num_workers = workers.size();
    auto random_worker = rand() % num_workers;

    for (int i = 0; i < num_workers; i++) {
        auto worker_id = (random_worker + i) % num_workers;
        if (auto job = workers[worker_id].steal_job(); job.has_value()) {
            return job;
        }
    }

    return std::nullopt;
}