#include "frame.h"

namespace Termy {

Frame::Frame(const std::string& initial)
    : buffer_(initial)
{}

auto Frame::write(const std::string& text,
                  std::optional<Color> fg,
                  std::optional<Color> bg) -> void
{
    auto has_color = fg.has_value() || bg.has_value();
    if (!has_color) {
        buffer_.append(text);
        return;
    }

    constexpr size_t max_escape_overhead = 16;
    buffer_.reserve(buffer_.size() + text.size() + max_escape_overhead);

    buffer_.append("\033[");
    if (fg.has_value()) {
        buffer_.append(std::to_string(to_foreground_code(*fg)));
    }
    if (fg.has_value() && bg.has_value()) {
        buffer_.append(";");
    }
    if (bg.has_value()) {
        buffer_.append(std::to_string(to_background_code(*bg)));
    }
    buffer_.append("m");
    buffer_.append(text);
    buffer_.append("\033[0m");
}

auto Frame::newline() -> void
{
    buffer_ += "\n";
}

auto Frame::to_string() const -> std::string
{
    return buffer_;
}

auto Frame::operator==(const Frame& other) const -> bool
{
    return buffer_ == other.buffer_;
}

}
