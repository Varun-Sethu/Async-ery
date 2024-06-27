#include <map>
#include <iostream>

#include "scheduler/circular_queue.h"
#include "scheduler/scheduler.h"

auto main() -> int {
    auto queue = CircularQueue(4);
    auto job_id = 0;
    auto job_results = std::map<int, bool>();
    auto scheduler = Async::Scheduler(0, {});

    std::srand(std::time(nullptr));

    auto get_job_id = [&job_id, &job_results]() -> int {
        job_results[job_id] = false;
        return job_id++;
    };

    for (auto i = 0; i < 1000; i++) {
        // generate a random amount of jobs
        auto num_jobs = std::rand() % 20;
        for (auto j = 0; j < num_jobs; j++) {
            auto job_id = get_job_id();
            queue.enqueue([&job_results, job_id](auto _){
                std::cout << "Running job " << job_id << std::endl;
                job_results[job_id] = true;
            });
        }


        // generate a random amount of jobs to run
        if (queue.size() == 0) { continue; }
        auto num_jobs_to_run = std::rand() % queue.size();
        for (size_t j = 0; j < num_jobs_to_run; j++) {
            auto job = queue.dequeue().value();
            job(scheduler.empty_context());
        }
    }

    // run all remaining jobs
    while (auto job = queue.dequeue()) {
        if (job.has_value()) { job.value()(scheduler.empty_context()); }
        else { break; }
    }

    // verify that all jobs were run
    for (auto [job_id, result] : job_results) {
        if (!result) {
            std::cout << "Job " << job_id << " was not run!" << std::endl;
            perror("Job was not run!");
        }
    }
}