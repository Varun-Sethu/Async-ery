#include "text_box.h"
#include "key.h"

#include <algorithm>
#include <cassert>
#include <span>

namespace Termy {

namespace {
struct Token {
    Token(std::string_view text, bool is_whitespace)
        : text_(text)
        , is_whitespace_(is_whitespace) {}

    std::string_view text_;
    bool             is_whitespace_;
};

// Tokenize takes a stream of characters and returns a vector of tokens.
// Whitespace charachters in the text are tokenised as individual tokens. Ie.
// the text: "   \n" gets tokenised as " ", " ", " ", "\n". Words are tokenised
// as a single token. Ie. the text: "Hello World" gets tokenised as "Hello", " ", "World".
auto tokenize(std::string_view text) -> std::vector<Token> {
    auto tokens = std::vector<Token>{};
    auto current_token_start = size_t{0};

    while (current_token_start < text.size()) {
        auto chr = text[current_token_start];
        auto is_whitespace = std::isspace(static_cast<unsigned char>(chr)) != 0;
        if (is_whitespace) {
            auto token_substr = text.substr(current_token_start, 1);
            tokens.emplace_back(token_substr, /* is_whitespace = */ true);
            current_token_start += 1;
            continue;
        }

        // Keep slurping up tokens so we can attempt to extend this token as far
        // as possible if it is a word. We stop slurping when we encounter a whitespace character.
        // or more specifically, when the next character is a whitespace character.
        auto current_token_end = current_token_start + 1;
        while (current_token_end < text.size()) {
            auto next_ch = text[current_token_end];
            auto next_is_whitespace = std::isspace(static_cast<unsigned char>(next_ch)) != 0;
            if (next_is_whitespace) {
                break;
            }

            current_token_end += 1;
        }

        auto token_substr = text.substr(current_token_start, current_token_end - current_token_start);
        tokens.emplace_back(token_substr, /* is_whitespace = */ false);

        current_token_start = current_token_end;
    }

    return tokens;
}

// map_buffer_cursor_to_screen_cursor will convert a cursor that corresponds to a position in the buffer to a cursor that corresponds to a position on the screen.
// This is required because the text_wrapping logic will often chomp off whitespace characters in order to fit the text within the available width.
// returns a tuple containing the line index and the column index on that line.
auto map_buffer_cursor_to_screen_cursor(
    std::size_t buffer_cursor,
    const std::vector<std::pair<size_t, std::string>>& wrapped_lines
) -> std::pair<std::size_t, std::size_t> {
    std::size_t line_index = 0;

    for (const auto& [line_start, line_text] : wrapped_lines) {
        auto cursor_within_line = buffer_cursor >= line_start &&
                                  buffer_cursor < line_start + line_text.size();
        if (cursor_within_line) {
            return { line_index, buffer_cursor - line_start };
        }

        line_index += 1;
    }

    // If no location is found, push the buffer cursor to the last
    // location in the string (last row, last col)
    return { wrapped_lines.size() - 1, wrapped_lines.back().second.size() };
}

struct ViewableRangeRenderingState {
    std::size_t viewable_line_range_start;
    std::size_t viewable_line_range_end;
    bool more_content_above_range;
    bool more_content_below_range;

    bool include_up_indicator = false;
    bool include_down_indicator = false;
};

// when rendering a textbox we want to include indicators to dictate if there
// there is more content the user has to scroll through. The only want to make
// room for this is if we chomp the viewable range to allow for this.
auto decorate_viewable_range_with_indicators(
    std::size_t cursor_line_position,
    ViewableRangeRenderingState render_state
) -> ViewableRangeRenderingState
{
    // We need to draw up and down indicators to indicate scrolling is required
    if (render_state.more_content_above_range) { render_state.include_up_indicator = true; }
    if (render_state.more_content_below_range) { render_state.include_down_indicator = true; }

    // These indicators take up space, so we need to adjust the viewable range
    // to account for them. There is a catch though... if the cursor is at the very
    // top or bottom of the viewable range, we need to be careful to ensure that
    // the cursor is not moved out of the viewable range.
    if (render_state.include_up_indicator) {
        if (cursor_line_position == render_state.viewable_line_range_start) {
            // Shrink the viewable range AT THE END to make room for the up indicator
            // note that the up indicator needs room for itself too, so we decrement by 2
            render_state.viewable_line_range_end -= 2;
            render_state.include_down_indicator   = true;
        } else {
            // Shrink the viewable range AT THE START to make room for the up indicator
            render_state.viewable_line_range_start += 1;
        }
    }

    if (render_state.include_down_indicator) {
        if (cursor_line_position == render_state.viewable_line_range_end - 1) {
            // Shrink the viewable range AT THE START to make room for the indicator
            // note that the up indicator needs room for itself too, so we increment by 2
            render_state.viewable_line_range_start += 2;
            render_state.include_up_indicator   = true;
        } else {
            // Shrink the viewable range AT THE END to make room for the up indicator
            render_state.viewable_line_range_end -= 1;
        }
    }

    return render_state;
}

auto pad_char_to_width(char indicator, std::size_t width) -> std::string {
    auto padding = (width > 1) ? (width - 1) / 2 : 0;
    auto indicator_ = std::string(padding, ' ') + indicator;
    auto remaining = width - padding - 1;
    if (remaining > 0) {
        indicator_ += std::string(remaining, ' ');
    }

    return indicator_;
}

}

// ==============================
// ==== Primary TextBox Impl ====
// ==============================
TextBox::TextBox(std::string text, TextBoxColors colors)
    : text_(std::move(text))
    , colors_(colors)
{}

auto TextBox::set_text(std::string text) -> void {
    text_ = std::move(text);
    cursor_state_ = CursorState {
        .cursor_raw_position = 0,
        .visible_lines_start = 0
    };
}

// wrap_text will convert some provided text into a vector of strings, each of which is no longer than max_width.
// This is mostly a utility for rendering. Alongside each string, the function returns where in the origional string
// the line "starts". This is required because the wrap text fn will delete whitespace if it lies on the boundary
// of a new line
auto TextBox::update_text_wrappings(const std::string& text, size_t max_width) -> void {
    if (text.empty()) { return; }

    cached_line_wrappings_.clear();
    if (max_width == 0) {
        return;
    }

    // current_line refers to the current line being built
    // while current_line_buffer_location refers to the location
    // in the origional un-tokenised text that the current line
    // "starts" at. The idea is that every time a new line is created, the line is annoteated
    // with the current_line_buffer_location that corresponds to it. Ie. where it started in
    // the origional text
    auto current_line = std::string{};
    auto current_line_buffer_location = size_t{0};
    auto current_buffer_location      = size_t{0};

    for (const auto& token : tokenize(text)) {
        // If this token triggers the start of any new lines, the line(s) that follow it will reference
        // the current_line + the token length
        auto is_new_line = token.text_ == "\n";
        if (is_new_line) {
            current_line += " ";
            cached_line_wrappings_.emplace_back(current_line_buffer_location, current_line);
            current_line.clear();

            current_buffer_location += token.text_.size();
            current_line_buffer_location = current_buffer_location;
            continue;
        }

        // Dont need to create a new line, token can be appended to this line
        auto token_fits_on_current_line = current_line.size() + token.text_.size() <= max_width;
        if (token_fits_on_current_line) {
            current_line += token.text_;
            current_buffer_location += token.text_.size();
            continue;
        }

        // If the token no longer fits we need to introduce a new line. There are a few distinct cases
        // here:
        //      Case 1: The current token we are looking at (that isnt a string token) is a whitespace charachter
        //              In that case, we discard the token and just introduce a new line
        //      Case 2: The current token isnt a whitespace charachter. In such a case, we need to introduce
        //              a new line and start a new line with the token text.
        //                  Case 2.1: The token is short enough to fit on a single line, so we just add it to the current line.
        //                  Case 2.2: The token is too long to fit on a single line, so we need to split it into multiple lines.
        // Case 1:
        if (token.is_whitespace_) {
            cached_line_wrappings_.emplace_back(current_line_buffer_location, current_line);
            current_line.clear();

            current_buffer_location += token.text_.size();
            current_line_buffer_location = current_buffer_location;
            continue;
        }

        // Case 2:
        if (!current_line.empty()) {
            cached_line_wrappings_.emplace_back(current_line_buffer_location, current_line);
            current_line_buffer_location = current_buffer_location;
            current_line.clear();
        }

        // Case 2.1:
        auto token_can_fit_on_line = token.text_.size() <= max_width;
        if (token_can_fit_on_line) {
            current_line += token.text_;
            current_buffer_location += token.text_.size();
        } else {
            // Case 2.2:
            for (std::size_t i = 0; i < token.text_.size(); i += max_width) {
                auto fragment_size = std::min(max_width, token.text_.size() - i);
                current_line += token.text_.substr(i, fragment_size);

                // Flush the current line to the line wrappings if it is now the max_width
                // ie. the next fragment wont fit on this line
                if (current_line.size() >= max_width) {
                    cached_line_wrappings_.emplace_back(current_line_buffer_location, current_line);
                    current_line.clear();

                    current_buffer_location += fragment_size;
                    current_line_buffer_location = current_buffer_location;
                } else {
                    current_buffer_location += fragment_size;
                }
            }
        }
    }

    cached_line_wrappings_.emplace_back(current_line_buffer_location, current_line);
}

auto TextBox::render(Frame frame) -> void {
    auto max_width = frame.content_max_width();
    auto max_height = frame.content_max_height();
    if (max_width == 0 || max_height == 0) {
        return;
    }

    update_text_wrappings(text_, max_width);
    const auto& wrapped_lines = cached_line_wrappings_;
    auto [cursor_line, cursor_col] = map_buffer_cursor_to_screen_cursor(cursor_state_.cursor_raw_position, wrapped_lines);

    // Store rendered position for up/down arrow navigation
    cursor_state_.rendered_row = cursor_line;
    cursor_state_.rendered_col = cursor_col;

    // Presently, there is a defined window on what the user can see within the textbox
    // if the cursor is outside that window then we need to update the window
    auto visible_lines_start = cursor_state_.visible_lines_start;
    auto visible_lines_end   = std::min(visible_lines_start + max_height, wrapped_lines.size());
    auto cursor_not_visible  = cursor_line < visible_lines_start || cursor_line >= visible_lines_end;
    if (cursor_not_visible) {
        auto cursor_above_window = cursor_line < visible_lines_start;
        auto cursor_below_window = cursor_line >= visible_lines_end;
        if (cursor_above_window) {
            visible_lines_start = cursor_line;
            visible_lines_end   = std::min(visible_lines_start + max_height, wrapped_lines.size());
        } else if (cursor_below_window) {
            // Shift visible_lines_start such that the cursor is at the bottom of the
            // window
            visible_lines_start = cursor_line - max_height + 1;
            visible_lines_end   = std::min(visible_lines_start + max_height, wrapped_lines.size());
        }
    }

    // Persist this so leaving the line that triggered a scroll doesnt "move" the window
    // back up.
    cursor_state_.visible_lines_start = visible_lines_start;

    // After computing the viewable range of lines, we need to determine if we should render an "up"
    // or "down" arrow indicating if there is more text. Documentation on how this is achieved
    // is within the function
    auto visible_text_area = decorate_viewable_range_with_indicators(
        cursor_line,
        ViewableRangeRenderingState {
            .viewable_line_range_start = visible_lines_start,
            .viewable_line_range_end = visible_lines_end,
            .more_content_above_range = visible_lines_start != 0,
            .more_content_below_range = visible_lines_end != wrapped_lines.size()
        }
    );

    if (visible_text_area.include_up_indicator) {
        frame.write(pad_char_to_width('^', max_width), Color::BrightBlack);
        frame.newline();
    }

    // Now we render the range of visible_lines_start to visible_lines_end
    for (auto line = visible_text_area.viewable_line_range_start; line < visible_text_area.viewable_line_range_end; line++) {
        const auto& [_, line_data] = wrapped_lines[line];
        auto cursor_is_at_this_line = line == cursor_line;
        if (!cursor_is_at_this_line) {
            frame.write(line_data);
            frame.newline();
            continue;
        }

        // If the cursor is in the middle of this line, we need to highlight the charachter the cursor is currently
        // at. This will require splitting the line in three, one before the char, one at, and one after
        if (cursor_col < line_data.size()) {
            auto line_before_cursor = line_data.substr(0, cursor_col);
            auto line_at_cursor = line_data.substr(cursor_col, 1);
            auto line_after_cursor = line_data.substr(cursor_col + 1);

            frame.write(line_before_cursor);
            frame.write(line_at_cursor, colors_.cursor_fg, colors_.cursor_bg);
            frame.write(line_after_cursor);
            frame.newline();
        } else {
            // We are at the end of a line! We should still render a cursor but just at the last charachter.
            frame.write(line_data);
            frame.write(" ", colors_.cursor_fg, colors_.cursor_bg);
            frame.newline();
        }
    }

    if (visible_text_area.include_down_indicator) {
        frame.write(pad_char_to_width('v', max_width), Color::BrightBlack);
        frame.newline();
    }
}

auto TextBox::on_special_key_press(Key key) -> void {
    if (key == Key::Left) {
        move_cursor_left();
    } else if (key == Key::Right) {
        move_cursor_right();
    } else if (key == Key::Up) {
        move_cursor_up();
    } else if (key == Key::Down) {
        move_cursor_down();
    } else if (key == Key::Backspace) {
        // Delete the character @ the cursor
        if (cursor_state_.cursor_raw_position > 0) {
            cursor_state_.cursor_raw_position--;
            text_.erase(cursor_state_.cursor_raw_position, 1);
        }
    } else if (key == Key::Enter) {
        // Insert a new line at the cursor position
        text_.insert(cursor_state_.cursor_raw_position, 1, '\n');
        cursor_state_.cursor_raw_position++;
    }
}

auto TextBox::on_char_key_press(char chr) -> void {
    text_.insert(cursor_state_.cursor_raw_position, 1, chr);
    cursor_state_.cursor_raw_position++;
}

auto TextBox::move_cursor_left() -> void {
    if (cursor_state_.cursor_raw_position > 0) {
        cursor_state_.cursor_raw_position--;
    }
}

auto TextBox::move_cursor_right() -> void {
    if (cursor_state_.cursor_raw_position < text_.size()) {
        cursor_state_.cursor_raw_position++;
    }
}

auto TextBox::move_cursor_up() -> void {
    auto cursor_at_top_of_area = cursor_state_.rendered_row == 0;
    if (cached_line_wrappings_.empty() || cursor_at_top_of_area) {
        return;
    }

    auto& [_, current_line_text] = cached_line_wrappings_[cursor_state_.rendered_row];
    auto cursor_at_end_of_line = cursor_state_.rendered_col >= current_line_text.size();

    // To determine the cursor's position in the text buffer we need to determine
    // how far into the "next line" it gets and where that next line starts.
    auto cursor_next_line = cursor_state_.rendered_row - 1;
    auto& [next_line_start, next_line_text] = cached_line_wrappings_[cursor_next_line];
    auto cursor_offset_in_next_line = cursor_at_end_of_line
        ? next_line_text.size()
        : std::min(cursor_state_.rendered_col, next_line_text.size());

    cursor_state_.cursor_raw_position = next_line_start + cursor_offset_in_next_line;
}

auto TextBox::move_cursor_down() -> void {
    auto cursor_at_bottom_of_area = cursor_state_.rendered_row >= cached_line_wrappings_.size() - 1;
    if (cached_line_wrappings_.empty() || cursor_at_bottom_of_area) {
        return;
    }

    auto& [_, current_line_text] = cached_line_wrappings_[cursor_state_.rendered_row];
    auto cursor_at_end_of_line = cursor_state_.rendered_col >= current_line_text.size();

    // To determine the cursor's position in the text buffer we need to determine
    // how far into the "next line" it gets and where that next line starts.
    auto cursor_next_line = cursor_state_.rendered_row + 1;
    auto& [next_line_start, next_line_text] = cached_line_wrappings_[cursor_next_line];
    auto cursor_offset_in_next_line = cursor_at_end_of_line
        ? next_line_text.size()
        : std::min(cursor_state_.rendered_col, next_line_text.size());

    cursor_state_.cursor_raw_position = next_line_start + cursor_offset_in_next_line;
}
}
