#pragma once

#include <vector>
#include <chrono>
#include <functional>

#include "scheduler/poll_source.h"
#include "termios.h"

#include "key.h"

namespace Termy {

// IKeyboardListener can be attached to a keyboard poll source to receive key press events.
// We extract this into its own interface instead of its own function as it allows for a listener
// to be removed from some poll source.
class IKeyboardListener {
public:
    virtual ~IKeyboardListener() = default;

    // on_special_key_press is invoked by the keyboard poll source whenever a
    // special key (arrows, enter, escape, etc.) is pressed.
    virtual auto on_special_key_press(Key key) -> void = 0;

    // on_char_key_press is invoked when a printable character is typed.
    // Default implementation does nothing - override in components that need text input.
    virtual auto on_char_key_press(char c) -> void { (void)c; }
};

// IKeyboardSource is an interface for a keyboard source that can be polled for key presses.
// Listeners can be added and removed from the source.
class IKeyboardSource {
public:
    virtual ~IKeyboardSource() = default;

    virtual auto add_listener(IKeyboardListener& listener) -> void = 0;
    virtual auto remove_listener(IKeyboardListener& listener) -> void = 0;
};

// KeyboardPollSource is a concrete implementation of IKeyboardSource that polls the keyboard for key presses.
// It uses the termios library to enable raw mode and disable echo.
class KeyboardPollSource : public Scheduler::IPollSource, public IKeyboardSource {
public:
    KeyboardPollSource();
    ~KeyboardPollSource() override;

    auto add_listener(IKeyboardListener& listener) -> void override;
    auto remove_listener(IKeyboardListener& listener) -> void override;

    auto poll_frequency() -> std::chrono::milliseconds override;
    auto poll() -> std::vector<Scheduler::Job> override;

private:
    auto create_special_key_listener_notification_jobs(Key key) -> std::vector<Scheduler::Job>;
    auto create_char_key_listener_notification_jobs(char c) -> std::vector<Scheduler::Job>;

    termios term_;
    termios original_term_;
    std::vector<IKeyboardListener*> listeners_;
};

}
