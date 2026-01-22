#pragma once

#include <chrono>
#include <vector>

#include <termios.h>

#include "scheduler/poll_source.h"

#include "key.h"

namespace Termy {

// IKeyboardListener can be attached to a keyboard poll source to receive key press events.
// We extract this into its own interface instead of its own function as it allows for a listener
// to be removed from some poll source.
class IKeyboardListener {
public:
    virtual ~IKeyboardListener() = default;

    IKeyboardListener(const IKeyboardListener&) = delete;
    auto operator=(const IKeyboardListener&) -> IKeyboardListener& = delete;
    IKeyboardListener(IKeyboardListener&&) = delete;
    auto operator=(IKeyboardListener&&) -> IKeyboardListener& = delete;

    // on_special_key_press is invoked by the keyboard poll source whenever a
    // special key (arrows, enter, escape, etc.) is pressed.
    virtual auto on_special_key_press(Key key) -> void = 0;

    // on_char_key_press is invoked when a printable character is typed.
    // Default implementation does nothing - override in components that need text input.
    virtual auto on_char_key_press(char character) -> void { (void)character; }

protected:
    IKeyboardListener() = default;
};

// IKeyboardSource is an interface for a keyboard source that can be polled for key presses.
// Listeners can be added and removed from the source.
class IKeyboardSource {
public:
    virtual ~IKeyboardSource() = default;

    IKeyboardSource(const IKeyboardSource&) = delete;
    auto operator=(const IKeyboardSource&) -> IKeyboardSource& = delete;
    IKeyboardSource(IKeyboardSource&&) = delete;
    auto operator=(IKeyboardSource&&) -> IKeyboardSource& = delete;

    virtual auto add_listener(IKeyboardListener& listener) -> void = 0;
    virtual auto remove_listener(IKeyboardListener& listener) -> void = 0;

protected:
    IKeyboardSource() = default;
};

// KeyboardPollSource is a concrete implementation of IKeyboardSource that polls the keyboard for key presses.
// It uses the termios library to enable raw mode and disable echo.
class KeyboardPollSource : public Scheduler::IPollSource, public IKeyboardSource {
public:
    KeyboardPollSource();
    ~KeyboardPollSource() override;

    KeyboardPollSource(const KeyboardPollSource&) = delete;
    auto operator=(const KeyboardPollSource&) -> KeyboardPollSource& = delete;
    KeyboardPollSource(KeyboardPollSource&&) = delete;
    auto operator=(KeyboardPollSource&&) -> KeyboardPollSource& = delete;

    auto add_listener(IKeyboardListener& listener) -> void override;
    auto remove_listener(IKeyboardListener& listener) -> void override;

    auto poll_frequency() -> std::chrono::milliseconds override;
    auto poll() -> std::vector<Scheduler::Job> override;

private:
    auto create_special_key_listener_notification_jobs(Key key) -> std::vector<Scheduler::Job>;
    auto create_char_key_listener_notification_jobs(char character) -> std::vector<Scheduler::Job>;

    termios term_;
    termios original_term_;
    std::vector<IKeyboardListener*> listeners_;
};

}
