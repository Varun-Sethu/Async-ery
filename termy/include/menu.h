#pragma once

#include <string>
#include <vector>

#include "color.h"
#include "frame.h"
#include "window_component.h"

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

class Menu : public IWindowComponent {
public:
    Menu(std::vector<MenuItem> items, MenuColors colors = {});

    // Implementation details for the IWindowComponent interface.
    // These methods allow Menu to be used within a Window.
    auto render(Frame frame) -> void override;
    auto on_special_key_press(Key key) -> void override;

private:
    auto move_menu_focus_up() -> void;
    auto move_menu_focus_down() -> void;

    // width() returns the width of the menu, this is basically the
    // maxmimum menu item label length plus some padding.
    auto width() const -> int;

private:
    std::vector<MenuItem> items_;
    MenuColors colors_;
    size_t focused_index_ = 0;

    static constexpr int HORIZONTAL_PADDING = 2;
};

}
