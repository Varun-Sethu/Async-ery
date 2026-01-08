#pragma once

namespace Termy {

class Frame;

// IRenderable is an object that the window class will consume and render. As such, the only method
// exposed by the interface is the render function itself.
class IRenderable {
public:
    virtual ~IRenderable() = default;

    // render() will write the contents of the IRenderable to some provided frame
    virtual auto render(Frame frame) -> void = 0;
};

}
