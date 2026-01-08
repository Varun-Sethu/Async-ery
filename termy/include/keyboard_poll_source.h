#pragma once

#include <vector>
#include <chrono>
#include <functional>

#include "scheduler/poll_source.h"
#include "termios.h"

#include "key.h"

namespace Termy {

class IKeyboardSource {
public:
    using KeyListener = std::function<void(Key)>;

    virtual ~IKeyboardSource() = default;
    virtual auto add_listener(const KeyListener& listener) -> void = 0;
};

class KeyboardPollSource : public Scheduler::IPollSource, public IKeyboardSource {
public:
    KeyboardPollSource();
    ~KeyboardPollSource() override;

    auto add_listener(const IKeyboardSource::KeyListener& listener) -> void override;

    auto poll_frequency() -> std::chrono::milliseconds override;
    auto poll() -> std::vector<Scheduler::Job> override;

private:
    auto create_listener_notification_jobs(Key key) -> std::vector<Scheduler::Job>;

    termios term_;
    termios original_term_;
    std::vector<IKeyboardSource::KeyListener> listeners_;
};

}
