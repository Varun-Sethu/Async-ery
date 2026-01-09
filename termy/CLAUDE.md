# CLAUDE.md

This file provides guidance to Claude Code when working with code in the termy/ directory.

**Note:** For build commands and general async_lib architecture, see the parent [../CLAUDE.md](../CLAUDE.md).

## Architecture Overview

**Termy** is a terminal UI (TUI) framework built on top of the async_lib. It provides composable UI components with keyboard input handling and multi-pane window layout.

### Directory Structure

```
termy/
├── include/                   # Public API headers
│   ├── color.h               # Color enum and ANSI code generation
│   ├── frame.h               # Frame class for sequential text writing
│   ├── key.h                 # Key enum for keyboard events
│   ├── keyboard_poll_source.h # Keyboard input polling (IPollSource)
│   ├── menu.h                # Menu component
│   ├── renderable.h          # IRenderable interface
│   ├── span_2d.h             # 2D view/span template
│   └── window.h              # Window with multi-pane layout
├── src/                      # Implementation files
│   ├── frame.cpp
│   ├── keyboard_poll_source.cpp
│   ├── menu.cpp
│   └── window.cpp
├── test/                     # Test files
│   ├── menu_test.cpp
│   ├── window_test.cpp
│   └── mocks/
│       └── mock_keyboard_source.h
└── main.cpp                  # Demo application
```

### Core Components

#### 1. Rendering System

**IRenderable** (`renderable.h`)
- Base interface for all UI components
- Single method: `render(Frame frame) -> void`

**Frame** (`frame.h`)
- Wraps a `Span2D<Cell>` view for sequential writing
- Tracks cursor position (row/col)
- Methods: `write(text, fg, bg)`, `newline()`, `set_content_width()`
- Handles centering and overflow clipping

**Window** (`window.h`)
- Container managing horizontal pane layout
- Owns 2D grid of `Cell` objects (char + fg/bg colors)
- `redraw_panes()` - Allocates width by percentage, calls render on each pane
- `to_string()` - Converts cell grid to ANSI-escaped output
- Optimizes output by grouping consecutive cells with same color

**Span2D<T>** (`span_2d.h`)
- Non-owning 2D view into underlying data
- Enables panes to write to isolated regions of shared cell buffer

#### 2. Color System

**Color** (`color.h`)
- Enum with 16 standard colors + bright variants + Default
- `to_foreground_code()` / `to_background_code()` - Convert to ANSI codes
- `ColouredString` - Builder class with fluent API for colored output

**Cell** (`window.h`)
- Struct: `char ch`, `Color fg`, `Color bg`
- Single character position in terminal grid

#### 3. Input Handling

**Key** (`key.h`)
- Enum: Up, Down, Left, Right, Enter, Escape, Backspace, Tab, Unknown

**IKeyboardSource** (`keyboard_poll_source.h`)
- Interface for keyboard event sources
- `add_listener(KeyListener)` - Register callback for key events

**KeyboardPollSource** (`keyboard_poll_source.h`)
- Implements both `IPollSource` (async_lib) and `IKeyboardSource`
- Configures terminal for raw input (non-canonical, non-echo, non-blocking)
- Parses ANSI escape sequences for arrow keys
- Polls every 5ms, creates scheduler jobs for listener notifications
- RAII: Restores terminal settings on destruction

#### 4. UI Components

**Menu** (`menu.h`)
- Implements `IRenderable`
- State: items (label + tooltip), focused index, colors
- Registers keyboard listeners for Up/Down navigation
- `render()` - Displays items with focus highlighting
- `move_up()` / `move_down()` - Navigate with boundary checks

### Component Interaction Flow

```
Terminal Input
    ↓
KeyboardPollSource::poll() [5ms interval]
    ↓
Creates Scheduler::Job → Notifies listeners
    ↓
Menu::move_up() / move_down() [updates focused_index]
    ↓
window.redraw_panes() [called from render loop]
    ↓
Window allocates Span2D views per pane percentage
    ↓
Menu::render(Frame) writes cells
    ↓
Window::to_string() → ANSI output to stdout
```

### Key Design Patterns

1. **Renderer Pattern**: Components implement IRenderable, Window orchestrates rendering
2. **View/Span Pattern**: Span2D provides non-owning views for isolated pane rendering
3. **Listener/Observer**: Components register callbacks with keyboard source
4. **Poll Integration**: KeyboardPollSource plugs into async_lib scheduler
5. **RAII**: Terminal state properly saved/restored by KeyboardPollSource

### Usage Example

```cpp
auto keyboard = std::make_shared<Termy::KeyboardPollSource>();
auto factory = TaskFactory(threads, {keyboard});

auto menu = Termy::Menu(items, *keyboard, colors);
auto window = Termy::Window(80, 24, {{menu, 0.5f}, {other, 0.5f}});

timer_source.periodic(33ms).for_each([&](auto) {
    window.redraw_panes();
    std::cout << "\033[H" << window.to_string() << std::flush;
});
```

### Testing

- GoogleTest framework
- MockKeyboardSource for simulating input without terminal
- Tests cover: menu rendering, navigation, window layout, pane percentages
