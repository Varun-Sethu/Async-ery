#include <algorithm>
#include <memory>

#include "io/io_request.h"
#include "io/types.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
//  - Note: The aio API requires us to use a C-style array for the buffer, regular C++ arrays are not compatible.
IO::ReadRequest::ReadRequest(Size nbytes, Offset offset)
    : buffer(new char[nbytes.size()]),
      nbytes(nbytes),
      file_offset(offset) {}

auto IO::ReadRequest::copy_buffer() const -> std::shared_ptr<char[]> {
    auto buff_size = nbytes.size();

    auto copy = std::shared_ptr<char[]>(new char[buff_size]);
    std::copy_n(buffer.get(), buff_size, copy.get());
    return copy;
}
// NOLINTEND(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)