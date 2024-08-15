#pragma once

#include <aio.h>
#include <memory>
#include <functional>

#include "io_request.h"

class InFlightAIORequest {
public:
    InFlightAIORequest(Async::IOReadRequest request, std::unique_ptr<struct aiocb> control_block)
        : request(request),
          control_block(std::move(control_block)) {}

    [[nodiscard]] auto underlying_request() -> Async::IOReadRequest const;
    [[nodiscard]] auto aio_control_block() -> std::shared_ptr<struct aiocb>&;
    auto is_completed() -> bool;

private:
    Async::IOReadRequest request;
    std::shared_ptr<struct aiocb> control_block;
};


class AIOManager {
public:
    static auto enqueue_and_start_read(FILE* fp, Async::IOReadRequest request) -> InFlightAIORequest;
};