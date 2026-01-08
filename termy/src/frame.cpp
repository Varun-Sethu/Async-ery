#include "frame.h"

namespace Termy {

Frame::Frame(Span2D<Cell> view)
    : view_(view)
{}

auto Frame::set_content_width(size_t width) -> void {
    content_width_ = width;
}

auto Frame::write(
    const std::string& text,
    std::optional<Color> fg,
    std::optional<Color> bg
) -> void {
    if (cursor_row_ >= view_.height) {
        return;
    }

    auto left_padding = std::max(std::size_t(0), view_.width - content_width_) / 2;
    auto effective_fg = fg.value_or(Color::Default);
    auto effective_bg = bg.value_or(Color::Default);

    for (auto i = 0u; i < text.size(); ++i) {
        auto col = left_padding + cursor_col_;
        auto column_overflows_frame = col >= view_.width;
        if (column_overflows_frame) {
            break;
        }

        view_.at(cursor_row_, col) = Cell {
            .ch = text[i],
            .fg = effective_fg,
            .bg = effective_bg
        };

        cursor_col_++;
    }
}

auto Frame::newline() -> void {
    ++cursor_row_;
    cursor_col_ = 0;
}

}
