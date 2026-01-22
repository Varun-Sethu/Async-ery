#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "color.h"
#include "frame.h"
#include "key.h"
#include "window_component.h"

namespace Termy {

struct TextBoxColors {
    Color text_fg = Color::White;
    Color text_bg = Color::Default;
    Color cursor_fg = Color::Black;
    Color cursor_bg = Color::White;
    Color scroll_indicator_fg = Color::BrightBlack;
    Color scroll_indicator_bg = Color::Default;
};

class TextBox : public IWindowComponent {
public:
    explicit TextBox(std::string text, TextBoxColors colors = {});

    auto set_text(std::string text) -> void;

    // Implementation details for the IWindowComponent interface.
    // These methods allow TextBox to be used within a Window.
    auto render(Frame frame) -> void override;
    auto on_special_key_press(Key key) -> void override;
    auto on_char_key_press(char character) -> void override;

private:
    using WrappedLines = std::vector<std::pair<size_t, std::string>>;

    // update_text_wrappings updates the wrapped_lines_ vector with the given text and max_width.
    // We need to cache the wrapped_lines_ vector to avoid recalculating it every time the text changes.
    // Additionally, the wrapped_lines_ vector is useful when moving the cursor up and down.
    auto update_text_wrappings(const std::string& text, size_t max_width) -> void;


    auto move_cursor_left() -> void;
    auto move_cursor_right() -> void;
    auto move_cursor_up() -> void;
    auto move_cursor_down() -> void;


    struct CursorState {
        // cursor_raw_position points to the charachter at which and edit will take effect.
        // it is essentially the position in the actual std::string that the cursor lines within
        std::size_t cursor_raw_position = 0;

        // visible_lines start defines what lines the user can currently see within
        // their textbox window. The range extends from [visibible_lines_start, visible_lines_start + max_frame_height)
        std::size_t visible_lines_start = 0;

        // rendered_row and rendered_col track where the cursor appears in the wrapped/rendered output.
        // These are updated during render() and used for up/down arrow navigation.
        std::size_t rendered_row = 0;
        std::size_t rendered_col = 0;
    };

    std::string text_;
    TextBoxColors colors_;
    CursorState cursor_state_;
    WrappedLines cached_line_wrappings_;
};

}
