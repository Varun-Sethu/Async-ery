#pragma once

#include <string>
#include <optional>

#include "color.h"

namespace Termy {

class Frame {
public:
    Frame() = default;
    explicit Frame(const std::string& initial);

    auto write(
        const std::string& text,
        std::optional<Color> fg = std::nullopt,
        std::optional<Color> bg = std::nullopt) -> void;

    auto newline() -> void;
    auto to_string() const -> std::string;

    auto operator==(const Frame& other) const -> bool;

private:
    std::string buffer_;
};

}
