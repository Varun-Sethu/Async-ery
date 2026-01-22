#include "window.h"

#include <algorithm>
#include <codecvt>
#include <cstddef>
#include <cstdint>
#include <locale>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// NOLINTNEXTLINE(misc-include-cleaner): fmt/format.h provides fmt::format which is used in to_string()
#include <fmt/format.h>

#include "color.h"
#include "frame.h"
#include "key.h"
#include "keyboard_poll_source.h"
#include "text_grid.h"
#include "window_component.h"

namespace Termy {
namespace {
    auto codepoint_to_utf8(uint32_t codepoint) -> std::string {
        auto convert = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>();
        return convert.to_bytes(codepoint);
    }
}

Window::Window(size_t width, size_t height, std::vector<WindowPane> panes, IKeyboardSource& keyboard_source)
    : width_(width)
    , height_(height)
    , cells_(height, std::vector<Cell>(width))
    , panes_(std::move(panes))
{
    keyboard_source.add_listener(*this);
}

// operator for subscribing and responding to keyboard events
// will be invoked by the keyboard source
auto Window::on_special_key_press(Key key) -> void {
    switch (pane_cursor_state_) {
        case PaneCursorState::NotFocused:
            return handle_key_press_when_no_pane_in_focus(key);
        case PaneCursorState::Focused:
            return handle_key_press_when_a_pane_is_in_focus(key);
    }
}

// When the window is unfocused the LEFT+RIGHT keys will cycle through the panes
// and pressing enter will select the currently hovered pane
auto Window::handle_key_press_when_no_pane_in_focus(Key key) -> void {
    switch (key) {
        case Key::Right: {
            // Progress to the next pane to focus on
            auto next_pane = (pane_cursor_ + 1) % panes_.size();
            pane_cursor_ = next_pane;
            break;
        }
        case Key::Left: {
            auto previous_pane = (pane_cursor_ - 1 + panes_.size()) % panes_.size();
            pane_cursor_ = previous_pane;
            break;
        }
        case Key::Enter:
            pane_cursor_state_ = PaneCursorState::Focused;
            break;
        default:
            // Not handled
            break;
    }
}

// When the window is focused the class only responds to the escape
// key. All other keys are forwarded to the currently focused pane
auto Window::handle_key_press_when_a_pane_is_in_focus(Key key) -> void {
    if (key == Key::Escape) {
        pane_cursor_state_ = PaneCursorState::NotFocused;
    } else {
        // Forward the key press to the currently focussed pane
        panes_[pane_cursor_].component.on_special_key_press(key);
    }
}

auto Window::on_char_key_press(char character) -> void {
    if (pane_cursor_state_ == PaneCursorState::Focused) {
        panes_[pane_cursor_].component.on_char_key_press(character);
    }
}

auto Window::redraw_panes() -> void {
    clear();

    auto current_col = size_t{0};

    for (auto i = size_t{0}; i < panes_.size(); ++i) {
        auto& pane = panes_[i];
        auto allocated_width = static_cast<size_t>(static_cast<float>(width_) * pane.percentage);
        auto remaining_width = width_ - current_col;
        auto pane_width = std::min(allocated_width, remaining_width);
        if (pane_width == 0) {
            continue;
        }

        auto span = TextGridSpan::from_grid(cells_, current_col, pane_width);

        auto pane_cursor_is_at_pane = (i == pane_cursor_);
        auto border_color = std::optional<Color>{std::nullopt};
        if (pane_cursor_is_at_pane) {
            border_color = (pane_cursor_state_ == PaneCursorState::Focused)
                ? Color::BrightWhite
                : Color::BrightBlack;
        }

        auto frame = Frame(span, border_color, pane.alignment);
        pane.component.render(frame);
        current_col += pane_width;
    }
}

auto Window::to_string() const -> std::string {
    auto produce_coloured_text_ansi = [](Color foreground, Color background, const std::string& text) -> std::string {
        if (text.empty()) {
            return {};
        }

        auto has_color = (foreground != Color::Default) || (background != Color::Default);
        if (has_color) {
            // NOLINTNEXTLINE(misc-include-cleaner): False positive - fmt/format.h is included at top of file
            return fmt::format("\033[{};{}m{}\033[0m",
                to_foreground_code(foreground),
                to_background_code(background),
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
            coloured_text_block += codepoint_to_utf8(cell.ch);
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
