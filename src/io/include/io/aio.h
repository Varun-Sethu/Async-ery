#pragma once

#include <aio.h>
#include <memory>
#include <functional>

#include "io_request.h"


namespace IO {
    class InFlightAIORequest {
    public:
        InFlightAIORequest(ReadRequest request, std::shared_ptr<struct aiocb> control_block)
            : request(std::move(request)),
              control_block(std::move(control_block)) {}

        [[nodiscard]] auto underlying_request() const -> ReadRequest;
        [[nodiscard]] auto aio_control_block() const -> const std::shared_ptr<struct aiocb>&;
        auto is_completed() -> bool;

    private:
        // Implementation note:
        // The control block is a shared_ptr as the intention is that it will be accessed by callbacks
        // upon a completed IO job. This forces us to use a shared_ptr as std::function and hence Scheduler::Job
        // require the lambda to be copy constructible. This is not possible with a lambda capturing a unique_ptr.
        ReadRequest request;
        std::shared_ptr<struct aiocb> control_block;
    };


    class AIOManager {
    public:
        static auto enqueue_and_start_read(FILE* file, ReadRequest request) -> InFlightAIORequest;
    };
}