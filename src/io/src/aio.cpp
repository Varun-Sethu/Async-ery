#include "io/aio.h"


auto InFlightAIORequest::is_completed() -> bool {
    return aio_error(control_block.get()) != EINPROGRESS;
}


auto InFlightAIORequest::underlying_request() -> Async::IOReadRequest const {
    return request;
}

auto InFlightAIORequest::aio_control_block() -> std::shared_ptr<struct aiocb>& {
    return control_block;
}


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