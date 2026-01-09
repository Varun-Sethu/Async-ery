#pragma once

#include <cstddef>
#include <vector>

namespace Termy {

template<typename T>
struct Span2D {
    std::vector<std::vector<T>>* data;
    size_t start_col;
    size_t width;
    size_t height;

    auto at(size_t row, size_t col) -> T& {
        return (*data)[row][start_col + col];
    }

    auto at(size_t row, size_t col) const -> const T& {
        return (*data)[row][start_col + col];
    }

    static auto from_vector(std::vector<std::vector<T>>& vec,
                            size_t start_col, size_t view_width) -> Span2D<T> {
        return Span2D<T>{
            .data = &vec,
            .start_col = start_col,
            .width = view_width,
            .height = vec.size()
        };
    }
};

}
