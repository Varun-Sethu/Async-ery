#include <utility>
#include <optional>
#include <vector>
#include <thread>

#include "scheduler/worker.h"
#include "scheduler/scheduling_context.h"
#include "scheduler/job.h"
#include "scheduler/job_queue.h"

Scheduler::JobWorker::JobWorker(Context worker_context, StealWork steal_work) : 
    worker_context(worker_context), 
    steal_work(std::move(steal_work)),
    worker_thread(std::nullopt) {}

auto Scheduler::JobWorker::start() -> bool {
    if (worker_thread.has_value()) {
        return false;
    }

    worker_thread = std::jthread([this](auto stop_token) {
        while (!stop_token.stop_requested()) {
            if (auto job = job_queue.dequeue(); job.has_value()) {
                job.value()(this->worker_context);
            } else {
                if (auto job = steal_work(); job.has_value()) {
                    job.value()(this->worker_context);
                }
            }
        }
    });   

    return true;
}

auto Scheduler::JobWorker::steal_job() -> std::optional<Job> { return job_queue.dequeue(); }
auto Scheduler::JobWorker::queue(Job job) -> void { queue(std::vector { std::move(job) });}
auto Scheduler::JobWorker::queue(std::vector<Job> jobs) -> void {
    for (auto& job : jobs) {
        job_queue.enqueue(std::move(job));
    }
}