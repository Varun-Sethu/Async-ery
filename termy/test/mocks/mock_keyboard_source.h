#pragma once

#include <algorithm>
#include <vector>

#include "keyboard_poll_source.h"
#include "key.h"

namespace Termy {

class MockKeyboardSource : public IKeyboardSource {
public:
    auto add_listener(IKeyboardListener& listener) -> void override {
        listeners_.push_back(&listener);
    }

    auto remove_listener(IKeyboardListener& listener) -> void override {
        listeners_.erase(
            std::remove(listeners_.begin(), listeners_.end(), &listener),
            listeners_.end()
        );
    }

    auto simulate_key(Key key) -> void {
        for (auto* listener : listeners_) {
            listener->on_special_key_press(key);
        }
    }

private:
    std::vector<IKeyboardListener*> listeners_;
};

}
