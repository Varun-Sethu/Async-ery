#pragma once

#include <cstddef>
#include <sys/types.h>

namespace IO {
    class Size {
    public:
        explicit Size(size_t size) : _size(size) {}
        [[nodiscard]] auto size() const -> size_t { return _size; }
    private:
        size_t _size;
    };
    
    
    class Offset {
    public:
        explicit Offset(off_t offset) : _offset(offset) {}
        [[nodiscard]] auto offset() const -> off_t { return _offset; }
    
    private:
        off_t _offset;
    };
}