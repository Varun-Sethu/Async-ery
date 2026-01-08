#pragma once

#include <string>
#include <vector>

#include "color.h"
#include "span_2d.h"
#include "renderable.h"

namespace Termy {

// Cell represents a single character in the terminal window.
// The window operates over a 2D grid of cells.
struct Cell {
    char ch = ' ';
    Color fg = Color::Default;
    Color bg = Color::Default;
};

// WindowPane represents a single pane in the terminal window.
// Each pane has an associated component and a percentage of the window's width.
// The window will layout the panes horizontally.
struct WindowPane {
    IRenderable& component;
    float percentage;
};


// Window represents a terminal window.
// It is composed of multiple panes, each with its own component.
// The window will layout the panes horizontally.
class Window {
public:
    Window(size_t width, size_t height, std::vector<WindowPane> panes);

    // redraw_panes will call render() on each of the panes and have them write
    // their content to the window's cells. Note that for an upto date view of
    // the window, redraw_panes() should be called after any changes to the window's
    // content.
    auto redraw_panes() -> void;

    // to_string will return a string representation of the window's content.
    // Note that for an upto date view of the window, to_string() should be called
    // after any changes to the window's content.
    [[nodiscard]] auto to_string() const -> std::string;

    // clear will clear the window's content.
    auto clear() -> void;

private:
    size_t width_;
    size_t height_;
    std::vector<std::vector<Cell>> cells_;
    std::vector<WindowPane> panes_;
};

}
