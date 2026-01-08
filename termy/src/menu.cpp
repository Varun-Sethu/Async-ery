#include "menu.h"
#include "key.h"

namespace Termy {

Menu::Menu(std::vector<MenuItem> items, IKeyboardSource& keyboard_source, MenuColors colors)
    : items_(std::move(items))
    , colors_(colors)
{
    keyboard_source.add_listener([this](Key key) {
        if (key == Key::Up) {
            move_up();
        } else if (key == Key::Down) {
            move_down();
        }
    });
}

auto Menu::render(Frame frame) -> void
{
    auto item_width = width();
    frame.set_content_width(static_cast<size_t>(item_width));

    for (size_t i = 0; i < items_.size(); ++i) {
        const auto is_focused = (i == focused_index_);
        const auto& item = items_[i];

        auto fg = is_focused ? colors_.focused_fg : colors_.unfocused_fg;
        auto bg = is_focused ? colors_.focused_bg : colors_.unfocused_bg;

        auto padded_label = " " + item.label;
        auto padding_needed = item_width - static_cast<int>(padded_label.length());
        for (auto p = 0; p < padding_needed; ++p) {
            padded_label += " ";
        }

        frame.write(padded_label, fg, bg);
        frame.newline();
    }
}

auto Menu::width() const -> int
{
    auto max_len = 0;
    for (const auto& item : items_) {
        auto len = static_cast<int>(item.label.length());
        if (len > max_len) {
            max_len = len;
        }
    }
    return max_len + HORIZONTAL_PADDING;
}

auto Menu::height() const -> int
{
    return static_cast<int>(items_.size());
}

auto Menu::move_up() -> void
{
    if (focused_index_ > 0) {
        --focused_index_;
    }
}

auto Menu::move_down() -> void
{
    if (focused_index_ < items_.size() - 1) {
        ++focused_index_;
    }
}

}
