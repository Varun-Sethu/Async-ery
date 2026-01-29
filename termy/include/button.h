#pragma once

#include <chrono>
#include <functional>
#include <string>

#include "async_lib/timer.h"
#include "color.h"
#include "frame.h"
#include "key.h"
#include "window_component.h"

namespace Termy {

struct ButtonColors {
    Color normal_fg = Color::Black;
    Color normal_bg = Color::White;
    Color pressed_fg = Color::White;
    Color pressed_bg = Color::Black;
};

class Button : public IWindowComponent {
public:
    using Callback = std::function<void()>;

    explicit Button(
        std::string label,
        Callback on_press,
        Timing::IPollSource& timing_source,
        ButtonColors colors = {}
    );

    auto render(Frame frame) -> void override;
    auto on_special_key_press(Key key) -> void override;

private:
    [[nodiscard]] auto width() const -> int;
    auto on_enter_release_detected() -> void;

    std::string label_;
    Callback on_press_;
    ButtonColors colors_;
    bool waiting_for_enter_release_ = false;

    Timer enter_release_timer_;

    static constexpr int HORIZONTAL_PADDING = 2;
    static constexpr auto ENTER_RELEASE_TIMER_DURATION = std::chrono::milliseconds(600);
};

}
