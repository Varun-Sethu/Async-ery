#include <algorithm>

#include "io/io_request.h"

Async::IOReadRequest::IOReadRequest(Size nbytes, Offset offset)
    : buffer(new char[nbytes.size()]),
      nbytes(nbytes),
      file_offset(offset) {}

auto Async::IOReadRequest::copy_buffer() const -> std::shared_ptr<char[]> {
    auto buff_size = nbytes.size();

    auto copy = std::shared_ptr<char[]>(new char[buff_size]);
    std::copy_n(buffer.get(), buff_size, copy.get());
    return copy;
}