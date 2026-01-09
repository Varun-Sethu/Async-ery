#pragma once

#include "keyboard_poll_source.h"

namespace Termy {

class Frame;

// IWindowComponents are components that can be used within a Window. The components
// are relatively straight forward and jsut require a mechanism for rendering
// and the ability to respond to key presses. If a component does not need a key press then
// they can discard any key press events.
class IWindowComponent : public IKeyboardListener {
public:
    virtual ~IWindowComponent() = default;

    virtual auto render(Frame frame) -> void = 0;
};

}
