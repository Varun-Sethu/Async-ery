#include <gtest/gtest.h>
#include <fmt/format.h>

#include "menu.h"
#include "frame.h"
#include "color.h"
#include "key.h"
#include "window.h"
#include "span_2d.h"
#include "mocks/mock_keyboard_source.h"

using Termy::Cell;
using Termy::Color;
using Termy::ColouredString;
using Termy::Key;
using Termy::MockKeyboardSource;
using Termy::Span2D;

static auto Focused(const std::string& text) -> std::string {
    return ColouredString(text).foreground(Color::Black).background(Color::White).ansi();
}

static auto Unfocused(const std::string& text) -> std::string {
    return ColouredString(text).foreground(Color::White).background(Color::Default).ansi();
}

static auto produce_coloured_text_ansi(Color fg, Color bg, const std::string& text) -> std::string {
    if (text.empty()) {
        return {};
    }

    auto has_color = (fg != Color::Default) || (bg != Color::Default);
    if (has_color) {
        return fmt::format("\033[{};{}m{}\033[0m",
            to_foreground_code(fg),
            to_background_code(bg),
            text);
    }
    return text;
}

static auto cells_to_string(const std::vector<std::vector<Cell>>& cells) -> std::string {
    std::string result;

    for (const auto& row : cells) {
        auto current_text_colour_fg = Color::Default;
        auto current_text_colour_bg = Color::Default;
        std::string coloured_text_block;

        for (const auto& cell : row) {
            auto color_changed = (cell.fg != current_text_colour_fg) || (cell.bg != current_text_colour_bg);
            if (color_changed) {
                result += produce_coloured_text_ansi(current_text_colour_fg, current_text_colour_bg, coloured_text_block);
                coloured_text_block.clear();
                current_text_colour_fg = cell.fg;
                current_text_colour_bg = cell.bg;
            }
            coloured_text_block += cell.ch;
        }

        result += produce_coloured_text_ansi(current_text_colour_fg, current_text_colour_bg, coloured_text_block);
        result += '\n';
    }

    return result;
}

TEST(MenuTest, RendersSingleItem) {
    auto items = std::vector<Termy::MenuItem>{{"Item 1", "tooltip"}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    constexpr size_t width = 8;
    constexpr size_t height = 1;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected = Focused(" Item 1 ") + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}

TEST(MenuTest, RendersMultipleItemsWithFocusOnFirst) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    constexpr size_t width = 10;
    constexpr size_t height = 3;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected =
        Focused(" Option A ") + "\n" +
        Unfocused(" Option B ") + "\n" +
        Unfocused(" Option C ") + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}

TEST(MenuTest, UsesCustomColors) {
    auto colors = Termy::MenuColors{
        .focused_fg = Color::Black,
        .focused_bg = Color::Cyan,
        .unfocused_fg = Color::White,
        .unfocused_bg = Color::Default
    };
    auto items = std::vector<Termy::MenuItem>{{"Test", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard, colors);

    constexpr size_t width = 6;
    constexpr size_t height = 1;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected =
        ColouredString(" Test ").foreground(Color::Black).background(Color::Cyan).ansi() + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}

TEST(MenuNavigationTest, MoveDownChangesFocusedItem) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    keyboard.simulate_key(Key::Down);

    constexpr size_t width = 10;
    constexpr size_t height = 3;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected =
        Unfocused(" Option A ") + "\n" +
        Focused(" Option B ") + "\n" +
        Unfocused(" Option C ") + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}

TEST(MenuNavigationTest, MoveUpChangesFocusedItem) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    keyboard.simulate_key(Key::Down);
    keyboard.simulate_key(Key::Down);

    constexpr size_t width = 10;
    constexpr size_t height = 3;
    auto cells1 = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span1 = Span2D<Cell>::from_vector(cells1, 0, width);
    auto frame1 = Termy::Frame(span1);
    menu.render(frame1);

    auto expected_after_downs =
        Unfocused(" Option A ") + "\n" +
        Unfocused(" Option B ") + "\n" +
        Focused(" Option C ") + "\n";
    EXPECT_EQ(cells_to_string(cells1), expected_after_downs);

    keyboard.simulate_key(Key::Up);

    auto cells2 = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span2 = Span2D<Cell>::from_vector(cells2, 0, width);
    auto frame2 = Termy::Frame(span2);
    menu.render(frame2);

    auto expected =
        Unfocused(" Option A ") + "\n" +
        Focused(" Option B ") + "\n" +
        Unfocused(" Option C ") + "\n";
    EXPECT_EQ(cells_to_string(cells2), expected);
}

TEST(MenuNavigationTest, MoveUpAtFirstItemDoesNothing) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    keyboard.simulate_key(Key::Up);

    constexpr size_t width = 10;
    constexpr size_t height = 2;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected =
        Focused(" Option A ") + "\n" +
        Unfocused(" Option B ") + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}

TEST(MenuNavigationTest, MoveDownAtLastItemDoesNothing) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    keyboard.simulate_key(Key::Down);
    keyboard.simulate_key(Key::Down);
    keyboard.simulate_key(Key::Down);

    constexpr size_t width = 10;
    constexpr size_t height = 2;
    auto cells = std::vector<std::vector<Cell>>(height, std::vector<Cell>(width));
    auto span = Span2D<Cell>::from_vector(cells, 0, width);
    auto frame = Termy::Frame(span);
    menu.render(frame);

    auto expected =
        Unfocused(" Option A ") + "\n" +
        Focused(" Option B ") + "\n";
    EXPECT_EQ(cells_to_string(cells), expected);
}
