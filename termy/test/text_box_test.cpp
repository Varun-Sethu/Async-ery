#include <cstddef>
#include <optional>
#include <string>

#include <gtest/gtest.h>

#include "color.h"
#include "frame.h"
#include "text_box.h"
#include "key.h"
#include "tui_test_helpers.h"

using Termy::Color;
using Termy::Key;
using Termy::ComponentAlignment;
using Termy::Testing::TestFrame;
using Termy::Testing::TextBorder;
using Termy::ColouredString;

static auto Cursor(const std::string& text) -> std::string {
    return ColouredString(text).foreground(Color::Black).background(Color::White).ansi();
}

static auto ScrollDownIndicator(size_t width) -> std::string {
    auto padding = (width > 1) ? (width - 1) / 2 : 0;
    auto indicator = std::string(padding, ' ') + "v";
    auto remaining = width - padding - 1;
    if (remaining > 0) {
        indicator += std::string(remaining, ' ');
    }
    return ColouredString(indicator).foreground(Color::BrightBlack).background(Color::Default).ansi();
}

static auto ScrollUpIndicator(size_t width) -> std::string {
    auto padding = (width > 1) ? (width - 1) / 2 : 0;
    auto indicator = std::string(padding, ' ') + "^";
    auto remaining = width - padding - 1;
    if (remaining > 0) {
        indicator += std::string(remaining, ' ');
    }
    return ColouredString(indicator).foreground(Color::BrightBlack).background(Color::Default).ansi();
}

static const auto DefaultColors = Termy::TextBoxColors{
    .text_fg = Color::White,
    .text_bg = Color::Default,
    .cursor_fg = Color::Black,
    .cursor_bg = Color::White
};

TEST(TextBoxTest, RendersSimpleText) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("Hello", DefaultColors);

    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(10, 3, {
        Cursor("H") + "ello"
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, RightArrowMovesCursorRight) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABC", DefaultColors);

    text_box.on_special_key_press(Key::Right);
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(10, 3, {
        "A" + Cursor("B") + "C"
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, LeftArrowMovesCursorLeft) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABC", DefaultColors);

    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(10, 3, { "AB" + Cursor("C") });
    EXPECT_FRAME_EQ(test_frame.to_string(), expected);

    text_box.on_special_key_press(Key::Left);
    text_box.render(frame);
    expected = TextBorder::PaddedContent(10, 3, { "A" + Cursor("B") + "C" });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, LeftArrowAtStartDoesNothing) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABC", DefaultColors);

    text_box.on_special_key_press(Key::Left);
    text_box.on_special_key_press(Key::Left);
    text_box.render(frame);
    auto expected = TextBorder::PaddedContent(10, 3, { Cursor("A") + "BC" });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, RightArrowAtEndShowsCursorOnSpace) {
    auto test_frame = TestFrame(10, 3);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("AB", DefaultColors);

    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.render(frame);
    auto expected = TextBorder::PaddedContent(10, 3, { "AB" + Cursor(" ") });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}


TEST(TextBoxTest, WrapsLongTextToFitWidth) {
    auto test_frame = TestFrame(7, 4);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJ", DefaultColors);

    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 4, {
        Cursor("A") + "BCDE",
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, ShowsDownArrowWhenContentOverflows) {
    auto test_frame = TestFrame(7, 4);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTUVWXYZ", DefaultColors);

    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 4, {
        Cursor("A") + "BCDE",
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, ShowsDownIndicatorWhenCursorNearBottom) {
    auto test_frame = TestFrame(7, 6);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTUVWXYZ", DefaultColors);

    for (int i = 0; i < 14; ++i) {
        text_box.on_special_key_press(Key::Right);
    }
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 6, {
        "ABCDE",
        "FGHIJ",
        "KLMN" + Cursor("O"),
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, UpArrowMovesCursorToPreviousLine) {
    auto test_frame = TestFrame(7, 6);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTUVWXYZ", DefaultColors);

    for (int i = 0; i < 14; ++i) {
        text_box.on_special_key_press(Key::Right);
    }
    text_box.render(frame);
    text_box.on_special_key_press(Key::Up);
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 6, {
        "ABCDE",
        "FGHI" + Cursor("J"),
        "KLMNO",
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, UpArrowAtTopDoesNothing) {
    auto test_frame = TestFrame(7, 4);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJ", DefaultColors);

    text_box.on_special_key_press(Key::Up);
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 4, {
        Cursor("A") + "BCDE",
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, CursorMovementAutoScrolls) {
    auto test_frame = TestFrame(7, 6);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTU", DefaultColors);

    for (int i = 0; i < 14; ++i) {
        text_box.on_special_key_press(Key::Right);
    }
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 6, {
        "ABCDE",
        "FGHIJ",
        "KLMN" + Cursor("O"),
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, ShowsUpIndicatorWhenScrolledDown) {
    auto test_frame = TestFrame(7, 7);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTUVWXYZ", DefaultColors);

    for (int i = 0; i < 24; ++i) {
        text_box.on_special_key_press(Key::Right);
    }
    text_box.render(frame);

    auto expected = TextBorder::PaddedContent(7, 7, {
        ScrollUpIndicator(5),
        "KLMNO",
        "PQRST",
        "UVWX" + Cursor("Y"),
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(TextBoxTest, EnterSplitsLongLine) {
    auto test_frame = TestFrame(7, 7);
    auto frame = test_frame.create_termy_frame(std::nullopt, ComponentAlignment::Left);
    auto text_box = Termy::TextBox("ABCDEFGHIJKLMNOPQRSTU", DefaultColors);

    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Enter);
    text_box.render(frame);
    auto expected = TextBorder::PaddedContent(7, 7, {
        "ABC",
        Cursor("D") + "EFGH",
        "IJKLM",
        "NOPQR",
        "STU"
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);


    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Right);
    text_box.on_special_key_press(Key::Enter);

    test_frame.clear();
    text_box.render(frame);
    expected = TextBorder::PaddedContent(7, 7, {
        "ABC",
        "DEFGH",
        "IJ",
        Cursor("K") + "LMNO",
        ScrollDownIndicator(5)
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}
