#pragma once

#include <memory>

namespace Async {
    class IOReadRequest {
        public:
            IOReadRequest(size_t nbytes, off_t offset)
                : buffer(new char[nbytes]),
                  nbytes(nbytes),
                  file_offset(offset) {}

            auto size() -> size_t { return nbytes; }
            auto offset() -> off_t { return file_offset; }
            auto underlying_buffer() -> std::shared_ptr<volatile char[]>& { return buffer; }

            auto copy_buffer() -> std::shared_ptr<char[]>;

        private:
            std::shared_ptr<volatile char[]> buffer;
            size_t nbytes;
            off_t file_offset;
    };
};




auto Async::IOReadRequest::copy_buffer() -> std::shared_ptr<char[]> {
    auto copy = std::shared_ptr<char[]>(new char[nbytes]);
    std::copy_n(buffer.get(), nbytes, copy.get());
    return copy;
}