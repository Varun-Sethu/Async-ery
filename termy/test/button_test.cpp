#include <chrono>

#include <gtest/gtest.h>

#include "button.h"
#include "color.h"
#include "key.h"
#include "scheduler/scheduling_context.h"
#include "tui_test_helpers.h"
#include "mocks/mock_timer_poll_source.h"

using Termy::Button;
using Termy::Color;
using Termy::Key;
using Termy::Testing::TestFrame;
using Termy::Testing::TextHighlight;
using Termy::Testing::TextBorder;

static auto GetButtonHighlight() -> TextHighlight {
    return {
        Color::White, Color::Black,
        Color::Black, Color::White
    };
}

TEST(ButtonTest, RendersButtonWithLabel) {
    auto timer_source = Mocks::MockTimerPollSource();
    auto press_count = 0;
    auto button = Button("Click Me", [&]() { ++press_count; }, timer_source);

    auto test_frame = TestFrame(12, 3);
    auto frame = test_frame.create_termy_frame();

    button.render(frame);

    auto expected = TextBorder::PaddedContent(12, 3, {
        GetButtonHighlight().Unfocused(" Click Me ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected);
}

TEST(ButtonTest, TestEnterKeyPress) {
    auto timer_source = Mocks::MockTimerPollSource();
    auto press_count = 0;
    auto button = Button("OK", [&]() { ++press_count; }, timer_source);

    button.on_special_key_press(Key::Enter);

    EXPECT_EQ(press_count, 1);

    auto test_frame = TestFrame(6, 3);
    auto frame = test_frame.create_termy_frame();
    button.render(frame);

    auto expected_pressed = TextBorder::PaddedContent(6, 3, {
        GetButtonHighlight().Focused(" OK ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected_pressed);

    auto jobs = timer_source.advance_time(std::chrono::milliseconds(600));
    for (auto& job : jobs) {
        job(Scheduler::Context::empty());
    }

    test_frame.clear();
    auto frame2 = test_frame.create_termy_frame();
    button.render(frame2);

    auto expected_normal = TextBorder::PaddedContent(6, 3, {
        GetButtonHighlight().Unfocused(" OK ")
    });

    EXPECT_FRAME_EQ(test_frame.to_string(), expected_normal);
}

TEST(ButtonTest, TestEnterKeyDebouncing) {
    auto timer_source = Mocks::MockTimerPollSource();
    // press_count tracks how many times the button callback has been invoked
    auto press_count = 0;
    auto button = Button("Submit", [&]() { ++press_count; }, timer_source);

    button.on_special_key_press(Key::Enter);
    EXPECT_EQ(press_count, 1);

    timer_source.advance_time(std::chrono::milliseconds(500));

    button.on_special_key_press(Key::Enter);
    EXPECT_EQ(press_count, 1);

    for (auto& job : timer_source.advance_time(std::chrono::milliseconds(100))) {
        job(Scheduler::Context::empty());
    }
    EXPECT_EQ(press_count, 1);

    for (auto& job : timer_source.advance_time(std::chrono::milliseconds(600))) {
        job(Scheduler::Context::empty());
    }
    EXPECT_EQ(press_count, 1);

    button.on_special_key_press(Key::Enter);
    EXPECT_EQ(press_count, 2);
}

TEST(ButtonTest, IgnoresNonEnterKeys) {
    auto timer_source = Mocks::MockTimerPollSource();
    auto press_count = 0;
    auto button = Button("Test", [&]() { ++press_count; }, timer_source);

    button.on_special_key_press(Key::Up);
    button.on_special_key_press(Key::Down);
    button.on_special_key_press(Key::Left);
    button.on_special_key_press(Key::Right);
    button.on_special_key_press(Key::Escape);
    button.on_special_key_press(Key::Tab);

    EXPECT_EQ(press_count, 0);
}

