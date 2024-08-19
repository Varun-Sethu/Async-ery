#include "io/io_request.h"

auto Async::IOReadRequest::copy_buffer() const -> std::shared_ptr<char[]> {
    auto copy = std::shared_ptr<char[]>(new char[nbytes]);
    std::copy_n(buffer.get(), nbytes, copy.get());
    return copy;
}