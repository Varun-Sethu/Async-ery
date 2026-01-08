#include "window.h"
#include "frame.h"

#include <fmt/format.h>

namespace Termy {

Window::Window(size_t width, size_t height, std::vector<WindowPane> panes)
    : width_(width)
    , height_(height)
    , cells_(height, std::vector<Cell>(width))
    , panes_(std::move(panes))
{
}

auto Window::redraw_panes() -> void {
    clear();

    auto current_col = size_t{0};

    for (auto& pane : panes_) {
        auto allocated_width = static_cast<size_t>(static_cast<float>(width_) * pane.percentage);
        auto remaining_width = width_ - current_col;
        auto pane_width = std::min(allocated_width, remaining_width);
        if (pane_width == 0) {
            continue;
        }

        auto span = Span2D<Cell>::from_vector(cells_, current_col, pane_width);
        pane.component.render(Frame(span));
        current_col += pane_width;
    }
}

auto Window::to_string() const -> std::string {
    auto produce_coloured_text_ansi = [](Color fg, Color bg, const std::string& text) -> std::string {
        if (text.empty()) {
            return {};
        }

        auto has_color = (fg != Color::Default) || (bg != Color::Default);
        if (has_color) {
            return fmt::format("\033[{};{}m{}\033[0m",
                to_foreground_code(fg),
                to_background_code(bg),
                text);
        }
        return text;
    };

    auto result = std::string{};

    for (const auto& row : cells_) {
        auto current_text_colour_fg = Color::Default;
        auto current_text_colour_bg = Color::Default;
        auto coloured_text_block = std::string{};

        for (const auto& cell : row) {
            auto color_changed = (cell.fg != current_text_colour_fg) || (cell.bg != current_text_colour_bg);
            if (color_changed) {
                result += produce_coloured_text_ansi(current_text_colour_fg, current_text_colour_bg, coloured_text_block);
                coloured_text_block.clear();
                current_text_colour_fg = cell.fg;
                current_text_colour_bg = cell.bg;
            }
            coloured_text_block += cell.ch;
        }

        result += produce_coloured_text_ansi(current_text_colour_fg, current_text_colour_bg, coloured_text_block);
        result += '\n';
    }

    return result;
}

auto Window::clear() -> void {
    for (auto& row : cells_) {
        for (auto& cell : row) {
            cell = Cell{};
        }
    }
}

}
