#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "color.h"
#include "text_grid.h"

namespace Termy {
enum class ComponentAlignment {
    Left,
    Center
};

class Frame {
public:
    Frame(TextGridSpan view, std::optional<Color> border_color, ComponentAlignment alignment = ComponentAlignment::Center);

    // Set the content width of the frame. This will adjust the width of the frame to the specified value.
    // If the specified width is greater than the view's width, the frame will be resized to fit the content.
    auto set_content_width(size_t width) -> void;

    // Write the contents of the IRenderable to the frame.
    auto write(
        const std::string& text,
        std::optional<Color> foreground = std::nullopt,
        std::optional<Color> background = std::nullopt) -> void;

    // newline indicates that the write cursor for the frame should move to the next line.
    // use this instead of \n when attempting to render a new line. Using \n will result in weirdly
    // rendered text.
    auto newline() -> void;

    // content_max_width & content_max_height indicates the maximum width and height of the content within the frame.
    // This is useful for determining the maximum width and height of the content within the frame.
    [[nodiscard]] auto content_max_width() const -> size_t;
    [[nodiscard]] auto content_max_height() const -> size_t;

private:
    auto draw_border(Color color) -> void;

    TextGridSpan view_;
    ComponentAlignment alignment_;
    size_t frame_content_row_cursor_ = 0;
    size_t frame_content_col_cursor_ = 0;
    size_t content_width_ = 0;

    size_t frame_content_row_start_ = 0;
    size_t frame_content_col_start_ = 0;
    size_t frame_content_max_width_ = 0;
    size_t frame_content_max_height_ = 0;
};

}
