#pragma once

#include <memory>

namespace Async {
    class IOReadRequest {
    public:
        IOReadRequest(size_t nbytes, off_t offset)
            : buffer(new char[nbytes]),
              nbytes(nbytes),
              file_offset(offset) {}

        auto size() -> size_t const { return nbytes; }
        auto offset() -> off_t const { return file_offset; }
        auto underlying_buffer() -> std::shared_ptr<volatile char[]>& { return buffer; }

        [[nodiscard]] auto copy_buffer() const -> std::shared_ptr<char[]>;

    private:
        std::shared_ptr<volatile char[]> buffer;
        size_t nbytes;
        off_t file_offset;
    };
};
