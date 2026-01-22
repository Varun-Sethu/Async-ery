#include "menu.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "frame.h"
#include "key.h"

namespace Termy {

Menu::Menu(std::vector<MenuItem> items, MenuColors colors)
    : items_(std::move(items))
    , colors_(colors)
{}

auto Menu::on_special_key_press(Key key) -> void {
    if (key == Key::Up) {
        move_menu_focus_up();
    } else if (key == Key::Down) {
        move_menu_focus_down();
    }
}

auto Menu::render(Frame frame) -> void
{
    auto item_width = width();
    frame.set_content_width(static_cast<size_t>(item_width));

    for (size_t i = 0; i < items_.size(); ++i) {
        const auto is_focused = (i == focused_index_);
        const auto& item = items_[i];

        auto foreground = is_focused ? colors_.focused_fg : colors_.unfocused_fg;
        auto background = is_focused ? colors_.focused_bg : colors_.unfocused_bg;

        auto padded_label = " " + item.label;
        auto padding_needed = item_width - static_cast<int>(padded_label.length());
        for (auto pad = 0; pad < padding_needed; ++pad) {
            padded_label += " ";
        }

        frame.write(padded_label, foreground, background);
        frame.newline();
    }
}

auto Menu::width() const -> int
{
    auto max_len = 0;
    for (const auto& item : items_) {
        auto len = static_cast<int>(item.label.length());
        max_len = std::max(len, max_len);
    }
    return max_len + HORIZONTAL_PADDING;
}

auto Menu::move_menu_focus_up() -> void
{
    if (focused_index_ > 0) {
        --focused_index_;
    }
}

auto Menu::move_menu_focus_down() -> void
{
    if (focused_index_ < items_.size() - 1) {
        ++focused_index_;
    }
}

}
