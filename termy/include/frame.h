#pragma once

#include <string>
#include <optional>

#include "color.h"
#include "window.h"

namespace Termy {

class Frame {
public:
    explicit Frame(Span2D<Cell> view);

    // Set the content width of the frame. This will adjust the width of the frame to the specified value.
    // If the specified width is greater than the view's width, the frame will be resized to fit the content.
    auto set_content_width(size_t width) -> void;

    // Write the contents of the IRenderable to the frame.
    auto write(
        const std::string& text,
        std::optional<Color> fg = std::nullopt,
        std::optional<Color> bg = std::nullopt) -> void;

    auto newline() -> void;

private:
    Span2D<Cell> view_;
    size_t cursor_row_ = 0;
    size_t cursor_col_ = 0;
    size_t content_width_ = 0;
};

}
