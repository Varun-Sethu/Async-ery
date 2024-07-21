#pragma once

#include <aio.h>
#include <memory>
#include <functional>

#include "io_request.h"


class InFlightAIORequest {
    public:
        InFlightAIORequest(Async::IOReadRequest request, std::unique_ptr<struct aiocb> control_block)
            : request(request), control_block(std::move(control_block)) {}

        auto is_completed() -> bool { return aio_error(control_block.get()) != EINPROGRESS; }
        auto underlying_request() -> Async::IOReadRequest const { return request; }
        auto aio_control_block() -> std::shared_ptr<struct aiocb>& { return control_block; }

    private:
        Async::IOReadRequest request;
        std::shared_ptr<struct aiocb> control_block;
};


class AIOManager {
    public:
        static auto enqueue_and_start_read(FILE* fp, Async::IOReadRequest request) -> InFlightAIORequest;
};


auto AIOManager::enqueue_and_start_read(FILE* fp, Async::IOReadRequest request) -> InFlightAIORequest {
    auto aio_request = InFlightAIORequest(
        request,
        std::make_unique<struct aiocb>((struct aiocb) {
            .aio_fildes = fileno(fp),
            .aio_lio_opcode = LIO_READ,
            .aio_buf = request.underlying_buffer().get(),
            .aio_nbytes = request.size(),
            .aio_offset = request.offset(),
        })
    );

    aio_read(aio_request.aio_control_block().get());
    return aio_request;
}