#pragma once

#include <string>
#include <vector>
#include <initializer_list>
#include <iostream>
#include <locale>
#include <codecvt>

#include <gtest/gtest.h>
#include <fmt/format.h>

#include "color.h"
#include "frame.h"
#include "text_grid.h"
#include "window.h"

#define EXPECT_FRAME_EQ(actual, expected) \
    { \
        auto actual_str = (actual); \
        auto expected_str = std::string(expected); \
        if (actual_str != expected_str) { \
            std::cerr << "\n\033[1;31m=== Actual ===\033[0m\n" << actual_str; \
            std::cerr << "\033[1;32m=== Expected ===\033[0m\n" << expected_str << "\n"; \
            EXPECT_TRUE(false) << "Frame content mismatch (see visual diff above)"; \
        } \
    }

namespace Termy::Testing {

namespace {
    auto codepoint_to_utf8(char32_t codepoint) -> std::string {
        auto convert = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>();
        return convert.to_bytes(codepoint);
    }

    auto visual_length(const std::string& text) -> size_t {
        size_t length = 0;
        bool in_escape = false;
        for (char c : text) {
            if (c == '\x1B') {
                in_escape = true;
            } else if (in_escape) {
                if (c == 'm') {
                    in_escape = false;
                }
            } else {
                length++;
            }
        }
        return length;
    }

    auto produce_coloured_text_ansi(Color fg, Color bg, const std::string& text) -> std::string {
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
    }
}

class TextPane {
public:
    TextPane() = default;

    explicit TextPane(std::vector<std::string> lines)
        : lines_(std::move(lines))
    {}

    operator std::string() const {
        auto result = std::string{};
        for (const auto& line : lines_) {
            result += line + "\n";
        }
        return result;
    }

    auto lines() const -> const std::vector<std::string>& {
        return lines_;
    }

    static auto StackedHorizontally(std::initializer_list<TextPane> panes) -> TextPane {
        if (panes.size() == 0) {
            return TextPane{};
        }

        auto max_height = size_t{0};
        for (const auto& pane : panes) {
            max_height = std::max(max_height, pane.lines().size());
        }

        auto result_lines = std::vector<std::string>{};
        for (size_t row = 0; row < max_height; ++row) {
            auto combined_line = std::string{};
            for (const auto& pane : panes) {
                if (row < pane.lines().size()) {
                    combined_line += pane.lines()[row];
                }
            }
            result_lines.push_back(combined_line);
        }

        return TextPane(std::move(result_lines));
    }

    friend auto operator==(const std::string& lhs, const TextPane& rhs) -> bool {
        return lhs == static_cast<std::string>(rhs);
    }

    friend auto operator==(const TextPane& lhs, const std::string& rhs) -> bool {
        return static_cast<std::string>(lhs) == rhs;
    }

private:
    std::vector<std::string> lines_;
};

class TestFrame {
public:
    TestFrame(size_t width, size_t height)
        : width_(width)
        , height_(height)
        , cells_(height, std::vector<Cell>(width))
    {}

    TestFrame(size_t width, size_t height, TextGrid cells)
        : width_(width)
        , height_(height)
        , cells_(std::move(cells))
    {}

    auto to_string() const -> std::string {
        auto result = std::string{};

        for (const auto& row : cells_) {
            auto current_fg = Color::Default;
            auto current_bg = Color::Default;
            auto coloured_text_block = std::string{};

            for (const auto& cell : row) {
                auto color_changed = (cell.fg != current_fg) || (cell.bg != current_bg);
                if (color_changed) {
                    result += produce_coloured_text_ansi(current_fg, current_bg, coloured_text_block);
                    coloured_text_block.clear();
                    current_fg = cell.fg;
                    current_bg = cell.bg;
                }
                coloured_text_block += codepoint_to_utf8(cell.ch);
            }

            result += produce_coloured_text_ansi(current_fg, current_bg, coloured_text_block);
            result += '\n';
        }

        return result;
    }

    auto create_termy_frame(
        std::optional<Color> border_color = std::nullopt,
        ComponentAlignment alignment = ComponentAlignment::Center
    ) -> Frame {
        auto span = TextGridSpan::from_grid(cells_, 0, width_);
        return Frame(span, border_color, alignment);
    }

    auto clear() -> void {
        for (auto& row : cells_) {
            for (auto& cell : row) {
                cell = Cell{};
            }
        }
    }

private:
    size_t width_;
    size_t height_;
    TextGrid cells_;
};

class TextHighlight {
public:
    TextHighlight(Color focused_fg, Color focused_bg, Color unfocused_fg, Color unfocused_bg)
        : focused_fg_(focused_fg)
        , focused_bg_(focused_bg)
        , unfocused_fg_(unfocused_fg)
        , unfocused_bg_(unfocused_bg)
    {}

    auto Focused(const std::string& text) const -> std::string {
        return ColouredString(text).foreground(focused_fg_).background(focused_bg_).ansi();
    }

    auto Unfocused(const std::string& text) const -> std::string {
        return ColouredString(text).foreground(unfocused_fg_).background(unfocused_bg_).ansi();
    }

private:
    Color focused_fg_;
    Color focused_bg_;
    Color unfocused_fg_;
    Color unfocused_bg_;
};

class TextBorder {
public:
    TextBorder(Color color)
        : color_(color)
    {}

    auto ColouredBorder(size_t width, std::initializer_list<std::string> content_lines) const -> TextPane {
        auto lines = std::vector<std::string>{};
        auto inner_width = (width >= 2) ? width - 2 : 0;

        auto horizontal_line = std::string{};
        for (size_t i = 0; i < inner_width; ++i) {
            horizontal_line += "\xE2\x94\x80";
        }

        auto top = std::string("\xE2\x94\x8C") + horizontal_line + std::string("\xE2\x94\x90");
        auto bottom = std::string("\xE2\x94\x94") + horizontal_line + std::string("\xE2\x94\x98");
        auto left_border = std::string("\xE2\x94\x82");
        auto right_border = std::string("\xE2\x94\x82");

        lines.push_back(ColouredString(top).foreground(color_).background(Color::Default).ansi());

        for (const auto& line : content_lines) {
            auto row = ColouredString(left_border).foreground(color_).background(Color::Default).ansi();
            row += line;
            row += ColouredString(right_border).foreground(color_).background(Color::Default).ansi();
            lines.push_back(row);
        }

        lines.push_back(ColouredString(bottom).foreground(color_).background(Color::Default).ansi());

        return TextPane(std::move(lines));
    }

    static auto PaddedContent(size_t width, size_t height, std::initializer_list<std::string> content_lines) -> TextPane {
        auto lines = std::vector<std::string>{};

        lines.push_back(std::string(width, ' '));

        for (const auto& line : content_lines) {
            auto padded_line = " " + line + " ";
            auto vis_len = visual_length(padded_line);
            if (vis_len < width) {
                padded_line += std::string(width - vis_len, ' ');
            }
            lines.push_back(padded_line);
        }

        auto remaining_rows = height - content_lines.size() - 2;
        for (size_t i = 0; i < remaining_rows; ++i) {
            lines.push_back(std::string(width, ' '));
        }

        lines.push_back(std::string(width, ' '));

        return TextPane(std::move(lines));
    }

private:
    Color color_;
};

}
