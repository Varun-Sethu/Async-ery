#pragma once

#include <vector>

#include "keyboard_poll_source.h"
#include "key.h"

namespace Termy {

class MockKeyboardSource : public IKeyboardSource {
public:
    auto add_listener(const KeyListener& listener) -> void override {
        listeners_.push_back(listener);
    }

    auto simulate_key(Key key) -> void {
        for (const auto& listener : listeners_) {
            listener(key);
        }
    }

private:
    std::vector<KeyListener> listeners_;
};

}
