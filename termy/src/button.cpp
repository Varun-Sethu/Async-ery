#include "button.h"

#include <cstddef>
#include <string>
#include <utility>

#include "frame.h"
#include "interface/timing/timer_poll_source_intf.h"
#include "key.h"

namespace Termy {

Button::Button(
    std::string label,
    Callback on_press,
    Timing::IPollSource& timing_source,
    ButtonColors colors
)
    : label_(std::move(label))
    , on_press_(std::move(on_press))
    , colors_(colors)
    , enter_release_timer_(
          ENTER_RELEASE_TIMER_DURATION,
          [this]() { on_enter_release_detected(); },
          timing_source
      )
{}

auto Button::on_special_key_press(Key key) -> void {
    if (key != Key::Enter) {
        return;
    }

    enter_release_timer_.reprogram();

    if (waiting_for_enter_release_) {
        return;
    }

    waiting_for_enter_release_ = true;
    on_press_();
}

auto Button::on_enter_release_detected() -> void {
    waiting_for_enter_release_ = false;
}

auto Button::render(Frame frame) -> void {
    auto button_width = width();
    frame.set_content_width(static_cast<size_t>(button_width));

    auto foreground = waiting_for_enter_release_ ? colors_.pressed_fg : colors_.normal_fg;
    auto background = waiting_for_enter_release_ ? colors_.pressed_bg : colors_.normal_bg;

    auto padded_label = " " + label_;
    auto padding_needed = button_width - static_cast<int>(padded_label.length());
    for (auto pad = 0; pad < padding_needed; ++pad) {
        padded_label += " ";
    }

    frame.write(padded_label, foreground, background);
}

auto Button::width() const -> int {
    return static_cast<int>(label_.length()) + HORIZONTAL_PADDING;
}

}
