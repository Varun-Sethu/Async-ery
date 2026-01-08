#include <gtest/gtest.h>

#include "menu.h"
#include "frame.h"
#include "color.h"
#include "key.h"
#include "mocks/mock_keyboard_source.h"

using Termy::Color;
using Termy::ColouredString;
using Termy::Key;
using Termy::MockKeyboardSource;

TEST(MenuTest, RendersSingleItem) {
    auto items = std::vector<Termy::MenuItem>{{"Item 1", "tooltip"}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);
    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Item 1 ").foreground(Color::Black).background(Color::White).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
}

TEST(MenuTest, RendersMultipleItemsWithFocusOnFirst) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);
    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::Black).background(Color::White).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option C ").foreground(Color::White).background(Color::Default).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
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
    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Test ").foreground(Color::Black).background(Color::Cyan).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
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

    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::Black).background(Color::White).ansi() + "\n" +
        ColouredString(" Option C ").foreground(Color::White).background(Color::Default).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
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

    auto frame_after_downs = Termy::Frame();
    menu.render(frame_after_downs);
    auto expected_after_downs = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option C ").foreground(Color::Black).background(Color::White).ansi() + "\n"
    );
    EXPECT_EQ(frame_after_downs, expected_after_downs);

    keyboard.simulate_key(Key::Up);

    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::Black).background(Color::White).ansi() + "\n" +
        ColouredString(" Option C ").foreground(Color::White).background(Color::Default).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
}

TEST(MenuNavigationTest, MoveUpAtFirstItemDoesNothing) {
    auto items = std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    keyboard.simulate_key(Key::Up);

    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::Black).background(Color::White).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::White).background(Color::Default).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
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

    auto frame = Termy::Frame();
    menu.render(frame);

    auto expected = Termy::Frame(
        ColouredString(" Option A ").foreground(Color::White).background(Color::Default).ansi() + "\n" +
        ColouredString(" Option B ").foreground(Color::Black).background(Color::White).ansi() + "\n"
    );
    EXPECT_EQ(frame, expected);
}
