#include <aio.h>
#include <memory>
#include <cstdio>
#include <cerrno>

#include "io/aio.h"
#include "io/io_request.h"


auto IO::InFlightAIORequest::is_completed() -> bool {
    return aio_error(control_block.get()) != EINPROGRESS;
}

auto parse_aio_error(int error_code) -> IO::AIOError {
    switch (error_code) {
        case EINPROGRESS:
            return IO::AIOError::InProgress;
        case ECANCELED:
            return IO::AIOError::Canceled;
        case EEXIST:
            return IO::AIOError::NoExist;
        default:
            return IO::AIOError::Unknown;
    }
}

auto IO::InFlightAIORequest::result() const -> IO::AIOResult<IO::ReadRequest> {
    auto aio_status = aio_error(control_block.get());
    if (aio_status == 0) {
        return { request };
    }

    auto parsed_status = parse_aio_error(aio_status);
    return { parsed_status };
}

auto IO::InFlightAIORequest::aio_control_block() const -> const std::shared_ptr<struct aiocb>& {
    return control_block;
}

auto IO::AIOManager::enqueue_and_start_read(FILE* file, IO::ReadRequest request) -> InFlightAIORequest {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
// We ignore the Wmissing-field-initializers" warning as it is perfectly safe to do so for AIO and the struct
// is relatively large.
    auto aio_request = InFlightAIORequest(
        request,
        std::make_shared<struct aiocb>(aiocb {
            .aio_fildes = file->_fileno,
            .aio_lio_opcode = LIO_READ,
            .aio_buf = request.underlying_buffer().get(),
            .aio_nbytes = request.size(),
            .aio_offset = request.offset(),
        })
    );
#pragma GCC diagnostic pop

    aio_read(aio_request.aio_control_block().get());
    return aio_request;
}
