#include <gtest/gtest.h>

#include "menu.h"
#include "key.h"
#include "tui_test_helpers.h"

using Termy::Color;
using Termy::Key;
using Termy::Testing::TestFrame;
using Termy::Testing::TextHighlight;
using Termy::Testing::TextBorder;

static const auto MenuHighlight = TextHighlight(
    Color::Black, Color::White,
    Color::White, Color::Default
);

TEST(MenuTest, RendersSingleItem) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Item 1", "tooltip"}
    });

    menu.render(frame);

    auto expected = TextBorder::PaddedContent(10, 3, {
        MenuHighlight.Focused(" Item 1 ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(MenuTest, RendersMultipleItemsWithFocusOnFirst) {
    auto test_frame = TestFrame(12, 5);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    });

    menu.render(frame);

    auto expected = TextBorder::PaddedContent(12, 5, {
        MenuHighlight.Focused(" Option A "),
        MenuHighlight.Unfocused(" Option B "),
        MenuHighlight.Unfocused(" Option C ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}


TEST(MenuNavigationTest, MoveDownChangesFocusedItem) {
    auto test_frame = TestFrame(12, 5);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    });

    menu.on_special_key_press(Key::Down);
    menu.render(frame);

    auto expected = TextBorder::PaddedContent(12, 5, {
        MenuHighlight.Unfocused(" Option A "),
        MenuHighlight.Focused(" Option B "),
        MenuHighlight.Unfocused(" Option C ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(MenuNavigationTest, MoveUpChangesFocusedItem) {
    auto test_frame = TestFrame(12, 5);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""},
        {"Option C", ""}
    });

    // First assert the state after pressing down a few times
    menu.on_special_key_press(Key::Down);
    menu.on_special_key_press(Key::Down);
    menu.render(frame);

    auto expected_after_downs = TextBorder::PaddedContent(12, 5, {
        MenuHighlight.Unfocused(" Option A "),
        MenuHighlight.Unfocused(" Option B "),
        MenuHighlight.Focused(" Option C ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected_after_downs);

    // Now press up and assert the new state, this should
    // have moved the focussed item to the middle
    menu.on_special_key_press(Key::Up);
    menu.render(frame);

    auto expected = TextBorder::PaddedContent(12, 5, {
        MenuHighlight.Unfocused(" Option A "),
        MenuHighlight.Focused(" Option B "),
        MenuHighlight.Unfocused(" Option C ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(MenuNavigationTest, MoveUpAtFirstItemDoesNothing) {
    auto test_frame = TestFrame(12, 4);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""}
    });

    menu.on_special_key_press(Key::Up);
    menu.render(frame);

    auto expected = TextBorder::PaddedContent(12, 4, {
        MenuHighlight.Focused(" Option A "),
        MenuHighlight.Unfocused(" Option B ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(MenuNavigationTest, MoveDownAtLastItemDoesNothing) {
    auto test_frame = TestFrame(12, 4);
    auto frame = test_frame.create_termy_frame();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"Option A", ""},
        {"Option B", ""}
    });

    menu.on_special_key_press(Key::Down);
    menu.on_special_key_press(Key::Down);
    menu.on_special_key_press(Key::Down);
    menu.render(frame);

    auto expected = TextBorder::PaddedContent(12, 4, {
        MenuHighlight.Unfocused(" Option A "),
        MenuHighlight.Focused(" Option B ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}
