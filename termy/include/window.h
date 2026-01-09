#pragma once

#include <string>
#include <vector>

#include "color.h"
#include "window_component.h"
#include "keyboard_poll_source.h"

namespace Termy {

// Cell represents a single character in the terminal window.
// The window operates over a 2D grid of cells.
struct Cell {
    char32_t ch = U' ';
    Color fg = Color::Default;
    Color bg = Color::Default;
};

// WindowPane represents a single pane in the terminal window.
// Each pane has an associated component and a percentage of the window's width.
// The window will layout the panes horizontally.
struct WindowPane {
    IWindowComponent& component;
    float percentage;
};

// Window takes a collection of panes and renders them. Each pane is given the percentage of the window's width.
// The window will layout the panes horizontally. Window also allows for keyboard input to be selectively given to each
// pane. The pane that the windows pane cursor currently points at can be either Focussed or NotFocused. If the pane is
// focused then it will recieve keyboard input. If it is not focused then it will not recieve keyboard input.
class Window : public IKeyboardListener {
public:
    Window(
        std::size_t width, std::size_t height,
        std::vector<WindowPane> panes,
        IKeyboardSource& keyboard_source);

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

    // Interface implementation for IKeyboardListener
    auto on_key_press(Key key) -> void override;

private:
    // Panes can either be focused or not focused. If a pane is not in focus
    // then keyboard input does not go towards the pane. If it is then keyboard
    // input is forwarded to the pane. Only the pane @ pane_cursor_ can ever
    // be in focus (but it can also be unfocused if enter was not pressed).
    enum class PaneCursorState {
        NotFocused,
        Focused
    };

    auto handle_key_press_when_no_pane_in_focus(Key key) -> void;
    auto handle_key_press_when_a_pane_is_in_focus(Key key) -> void;

    size_t width_;
    size_t height_;
    std::vector<std::vector<Cell>> cells_;
    std::vector<WindowPane> panes_;

    // The pane_cursor_ is the index of the pane that the user is currently interacting with.
    // If the user presses enter than the pane @ pane_cursor_ will be focused. If they press
    // escape then it is no longer in focus.
    size_t pane_cursor_ = 0;
    PaneCursorState pane_cursor_state_ = PaneCursorState::NotFocused;
};

}
