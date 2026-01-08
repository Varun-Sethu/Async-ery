#pragma once

#include <string>
#include <optional>

namespace Termy {

enum class Color {
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Magenta = 5,
    Cyan = 6,
    White = 7,
    BrightBlack = 8,
    BrightRed = 9,
    BrightGreen = 10,
    BrightYellow = 11,
    BrightBlue = 12,
    BrightMagenta = 13,
    BrightCyan = 14,
    BrightWhite = 15,
    Default = 16
};

constexpr auto to_foreground_code(Color c) -> int {
    if (c == Color::Default) {
        return 39;
    }
    auto val = static_cast<int>(c);
    return (val < 8) ? (30 + val) : (90 + val - 8);
}

constexpr auto to_background_code(Color c) -> int {
    if (c == Color::Default) {
        return 49;
    }
    auto val = static_cast<int>(c);
    return (val < 8) ? (40 + val) : (100 + val - 8);
}

class ColouredString {
public:
    explicit ColouredString(std::string text)
        : text_(std::move(text))
    {}

    auto foreground(Color c) -> ColouredString& {
        fg_ = c;
        return *this;
    }

    auto background(Color c) -> ColouredString& {
        bg_ = c;
        return *this;
    }

    auto ansi() const -> std::string {
        auto result = std::string();
        auto has_color = fg_.has_value() || bg_.has_value();

        if (has_color) {
            result += "\033[";
            if (fg_.has_value()) {
                result += std::to_string(to_foreground_code(*fg_));
            }
            if (fg_.has_value() && bg_.has_value()) {
                result += ";";
            }
            if (bg_.has_value()) {
                result += std::to_string(to_background_code(*bg_));
            }
            result += "m";
        }

        result += text_;

        if (has_color) {
            result += "\033[0m";
        }

        return result;
    }

private:
    std::string text_;
    std::optional<Color> fg_;
    std::optional<Color> bg_;
};

}
