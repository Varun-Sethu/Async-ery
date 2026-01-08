#pragma once

#include <string>
#include <vector>

#include "color.h"
#include "frame.h"
#include "keyboard_poll_source.h"

namespace Termy {

struct MenuColors {
    Color focused_fg = Color::Black;
    Color focused_bg = Color::White;
    Color unfocused_fg = Color::White;
    Color unfocused_bg = Color::Default;
};

struct MenuItem {
    std::string label;
    std::string tooltip;
};

class Menu {
public:
    Menu(std::vector<MenuItem> items, IKeyboardSource& keyboard_source, MenuColors colors = {});

    auto render(Frame& frame) -> void;

private:
    auto move_up() -> void;
    auto move_down() -> void;
    auto width() const -> int;
    auto height() const -> int;

private:
    std::vector<MenuItem> items_;
    MenuColors colors_;
    size_t focused_index_ = 0;

    static constexpr int HORIZONTAL_PADDING = 2;
};

}
