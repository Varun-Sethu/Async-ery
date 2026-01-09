#include "frame.h"

namespace Termy {

Frame::Frame(Span2D<Cell> view, std::optional<Color> border_color)
    : view_(view)
    , frame_content_row_start_(1)
    , frame_content_col_start_(1)
    , frame_content_max_width_((view.width >= 2) ? view.width - 2 : 0)
    , frame_content_max_height_((view.height >= 2) ? view.height - 2 : 0)
{
    if (border_color.has_value()) {
        draw_border(border_color.value());
    }
}

auto Frame::set_content_width(size_t width) -> void {
    content_width_ = width;
}

auto Frame::write(
    const std::string& text,
    std::optional<Color> fg,
    std::optional<Color> bg
) -> void {
    // TODO: improve failure semantics (perhaps log or return error)
    auto contains_newline = text.find('\n') != std::string::npos;
    if (contains_newline) {
        return;
    }

    if (frame_content_row_cursor_ >= frame_content_max_height_) {
        return;
    }

    auto left_padding = (frame_content_max_width_ > content_width_) ? (frame_content_max_width_ - content_width_) / 2 : size_t{0};
    auto fg_color = fg.value_or(Color::Default);
    auto bg_color = bg.value_or(Color::Default);

    for (auto i = 0u; i < text.size(); ++i) {
        auto col = left_padding + frame_content_col_cursor_;
        auto column_overflows_frame = col >= frame_content_max_width_;
        if (column_overflows_frame) {
            break;
        }

        view_.at(frame_content_row_start_ + frame_content_row_cursor_, frame_content_col_start_ + col) = Cell {
            .ch = static_cast<char32_t>(text[i]),
            .fg = fg_color,
            .bg = bg_color
        };

        frame_content_col_cursor_++;
    }
}

auto Frame::newline() -> void {
    ++frame_content_row_cursor_;
    frame_content_col_cursor_ = 0;
}

auto Frame::draw_border(Color color) -> void {
    constexpr auto TOP_LEFT = U'┌';
    constexpr auto TOP_RIGHT = U'┐';
    constexpr auto BOTTOM_LEFT = U'└';
    constexpr auto BOTTOM_RIGHT = U'┘';
    constexpr auto HORIZONTAL = U'─';
    constexpr auto VERTICAL = U'│';

    auto border_too_small = (view_.width < 3) || (view_.height < 3);
    if (border_too_small) {
        return;
    }

    auto fg = color;
    auto bg = Color::Default;

    view_.at(0, 0) = Cell{.ch = TOP_LEFT, .fg = fg, .bg = bg};
    view_.at(0, view_.width - 1) = Cell{.ch = TOP_RIGHT, .fg = fg, .bg = bg};
    view_.at(view_.height - 1, 0) = Cell{.ch = BOTTOM_LEFT, .fg = fg, .bg = bg};
    view_.at(view_.height - 1, view_.width - 1) = Cell{.ch = BOTTOM_RIGHT, .fg = fg, .bg = bg};

    for (auto col = size_t{1}; col < view_.width - 1; ++col) {
        view_.at(0, col) = Cell{.ch = HORIZONTAL, .fg = fg, .bg = bg};
        view_.at(view_.height - 1, col) = Cell{.ch = HORIZONTAL, .fg = fg, .bg = bg};
    }

    for (auto row = size_t{1}; row < view_.height - 1; ++row) {
        view_.at(row, 0) = Cell{.ch = VERTICAL, .fg = fg, .bg = bg};
        view_.at(row, view_.width - 1) = Cell{.ch = VERTICAL, .fg = fg, .bg = bg};
    }
}

}
