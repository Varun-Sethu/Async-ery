#pragma once

#include <cstddef>
#include <vector>

#include "color.h"

namespace Termy {

// Cell represents a single character in the terminal window.
// The window operates over a 2D grid of cells.
struct Cell {
    char32_t ch = U' ';
    Color fg = Color::Default;
    Color bg = Color::Default;
};

using TextGrid = std::vector<std::vector<Cell>>;

class TextGridSpan {
public:
    static auto from_grid(TextGrid& grid, size_t start_col, size_t view_width) -> TextGridSpan {
        return {&grid, start_col, view_width, grid.size()};
    }

    auto at(size_t row, size_t col) -> Cell& {
        return (*data_)[row][start_col_ + col];
    }

    [[nodiscard]] auto at(size_t row, size_t col) const -> const Cell& {
        return (*data_)[row][start_col_ + col];
    }

    [[nodiscard]] auto width() const -> size_t { return width_; }
    [[nodiscard]] auto height() const -> size_t { return height_; }

private:
    TextGridSpan(TextGrid* data, size_t start_col, size_t width, size_t height)
        : data_(data)
        , start_col_(start_col)
        , width_(width)
        , height_(height)
    {}

    TextGrid* data_;
    size_t start_col_;
    size_t width_;
    size_t height_;
};

}
