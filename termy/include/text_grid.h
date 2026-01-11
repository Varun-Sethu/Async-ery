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

struct TextGridSpan {
    TextGrid* data;
    size_t start_col;
    size_t width;
    size_t height;

    auto at(size_t row, size_t col) -> Cell& {
        return (*data)[row][start_col + col];
    }

    auto at(size_t row, size_t col) const -> const Cell& {
        return (*data)[row][start_col + col];
    }

    static auto from_grid(TextGrid& grid, size_t start_col, size_t view_width) -> TextGridSpan {
        return TextGridSpan{
            .data = &grid,
            .start_col = start_col,
            .width = view_width,
            .height = grid.size()
        };
    }
};

}
