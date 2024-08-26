#pragma once

#include <memory>

#include "io/types.h"

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
//  - Note: The aio API requires us to use a C-style array for the buffer, regular C++ arrays are not compatible.
namespace IO {
    class ReadRequest {
    public:
        ReadRequest(Size nbytes, Offset offset);

        [[nodiscard]] auto size() const -> size_t { return nbytes.size(); }
        [[nodiscard]] auto offset() const -> off_t { return file_offset.offset(); }
        [[nodiscard]] auto underlying_buffer() -> std::shared_ptr<volatile char[]>& { return buffer; }
        [[nodiscard]] auto copy_buffer() const -> std::shared_ptr<char[]>;

    private:
        std::shared_ptr<volatile char[]> buffer;
        Size nbytes;
        Offset file_offset;
    };
}
// NOLINTEND(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
