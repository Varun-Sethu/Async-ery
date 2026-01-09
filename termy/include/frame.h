#pragma once

#include <string>
#include <optional>

#include "color.h"
#include "span_2d.h"
#include "window.h"

namespace Termy {

class Frame {
public:
    Frame(Span2D<Cell> view, std::optional<Color> border_color);

    // Set the content width of the frame. This will adjust the width of the frame to the specified value.
    // If the specified width is greater than the view's width, the frame will be resized to fit the content.
    auto set_content_width(size_t width) -> void;

    // Write the contents of the IRenderable to the frame.
    auto write(
        const std::string& text,
        std::optional<Color> fg = std::nullopt,
        std::optional<Color> bg = std::nullopt) -> void;

    // newline indicates that the write cursor for the frame should move to the next line.
    // use this instead of \n when attempting to render a new line. Using \n will result in weirdly
    // rendered text.
    auto newline() -> void;

private:
    auto draw_border(Color color) -> void;

    Span2D<Cell> view_;
    size_t frame_content_row_cursor_ = 0;
    size_t frame_content_col_cursor_ = 0;
    size_t content_width_ = 0;

    size_t frame_content_row_start_;
    size_t frame_content_col_start_;
    size_t frame_content_max_width_;
    size_t frame_content_max_height_;
};

}
