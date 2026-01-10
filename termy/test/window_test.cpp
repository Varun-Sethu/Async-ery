#include <gtest/gtest.h>

#include "window.h"
#include "frame.h"
#include "menu.h"
#include "key.h"
#include "mocks/mock_keyboard_source.h"
#include "tui_test_helpers.h"

using Termy::Color;
using Termy::Key;
using Termy::ComponentAlignment;
using Termy::MockKeyboardSource;
using Termy::Testing::TextHighlight;
using Termy::Testing::TextBorder;
using Termy::Testing::TextPane;

static const auto MenuHighlight = TextHighlight(
    Color::Black, Color::White,
    Color::White, Color::Default
);

static const auto PaneBorder = TextBorder(Color::BrightBlack);
static const auto CandidateBorder = TextBorder(Color::BrightBlack);
static const auto FocusedBorder = TextBorder(Color::BrightWhite);

TEST(WindowTest, RendersSinglePaneFullWidth) {
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{{"Item", ""}});
    auto window = Termy::Window(8, 3, {
        {.component = menu, .percentage = 1.0f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = PaneBorder.ColouredBorder(8, {
        MenuHighlight.Focused(" Item ")
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowTest, RendersTwoPanesSideBySide) {
    auto keyboard = MockKeyboardSource();
    auto menu_left = Termy::Menu(std::vector<Termy::MenuItem>{{"L", ""}});
    auto menu_right = Termy::Menu(std::vector<Termy::MenuItem>{{"R", ""}});
    auto window = Termy::Window(10, 3, {
        {.component = menu_left, .percentage = 0.5f, .alignment = ComponentAlignment::Center},
        {.component = menu_right, .percentage = 0.5f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = TextPane::StackedHorizontally({
        PaneBorder.ColouredBorder(5, { MenuHighlight.Focused(" L ") }),
        TextBorder::PaddedContent(5, 3, { MenuHighlight.Focused(" R ") })
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowTest, CalculatesPercentageWidthsCorrectly) {
    auto keyboard = MockKeyboardSource();
    auto menu_left = Termy::Menu(std::vector<Termy::MenuItem>{{"X", ""}});
    auto menu_right = Termy::Menu(std::vector<Termy::MenuItem>{{"Y", ""}});
    auto window = Termy::Window(10, 3, {
        {.component = menu_left, .percentage = 0.5f, .alignment = ComponentAlignment::Center},
        {.component = menu_right, .percentage = 0.5f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = TextPane::StackedHorizontally({
        PaneBorder.ColouredBorder(5, { MenuHighlight.Focused(" X ") }),
        TextBorder::PaddedContent(5, 3, { MenuHighlight.Focused(" Y ") })
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowTest, CentersContentInWiderPane) {
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{{"Hi", ""}});
    auto window = Termy::Window(10, 3, {
        {.component = menu, .percentage = 1.0f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = PaneBorder.ColouredBorder(10, {
        "  " + MenuHighlight.Focused(" Hi ") + "  "
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowTest, MultipleRowsRenderedCorrectly) {
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{
        {"One", ""},
        {"Two", ""}
    });
    auto window = Termy::Window(7, 4, {
        {.component = menu, .percentage = 1.0f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = PaneBorder.ColouredBorder(7, {
        MenuHighlight.Focused(" One "),
        MenuHighlight.Unfocused(" Two ")
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowTest, TruncatesContentThatExceedsPaneWidth) {
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(std::vector<Termy::MenuItem>{{"Hello", ""}});
    auto window = Termy::Window(6, 3, {
        {.component = menu, .percentage = 1.0f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    window.redraw_panes();
    auto expected = PaneBorder.ColouredBorder(6, {
        MenuHighlight.Focused(" Hel")
    });

    EXPECT_EQ(window.to_string(), expected);
}

TEST(WindowFocusTest, ArrowKeysMoveCandidateAndEnterFocuses) {
    auto keyboard = MockKeyboardSource();
    auto menu_a = Termy::Menu(std::vector<Termy::MenuItem>{{"A", ""}});
    auto menu_b = Termy::Menu(std::vector<Termy::MenuItem>{{"B", ""}});
    auto menu_c = Termy::Menu(std::vector<Termy::MenuItem>{{"C", ""}});
    auto window = Termy::Window(15, 3, {
        {.component = menu_a, .percentage = 0.34f, .alignment = ComponentAlignment::Center},
        {.component = menu_b, .percentage = 0.34f, .alignment = ComponentAlignment::Center},
        {.component = menu_c, .percentage = 0.34f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    auto HOVERING_OVER = [&](const std::string& item) { return CandidateBorder.ColouredBorder(5, { item }); };
    auto FOCUSED = [&](const std::string& item) { return FocusedBorder.ColouredBorder(5, { item }); };
    auto UNFOCUSED = [&](const std::string& item) { return TextBorder::PaddedContent(5, 3, { item }); };

    window.redraw_panes();
    auto initial_expected = TextPane::StackedHorizontally({
        HOVERING_OVER(MenuHighlight.Focused(" A ")),
        UNFOCUSED(MenuHighlight.Focused(" B ")),
        UNFOCUSED(MenuHighlight.Focused(" C "))
    });
    EXPECT_EQ(window.to_string(), std::string(initial_expected));

    keyboard.simulate_key(Key::Right);
    window.redraw_panes();
    auto after_right_expected = TextPane::StackedHorizontally({
        UNFOCUSED(MenuHighlight.Focused(" A ")),
        HOVERING_OVER(MenuHighlight.Focused(" B ")),
        UNFOCUSED(MenuHighlight.Focused(" C "))
    });
    EXPECT_EQ(window.to_string(), std::string(after_right_expected));

    keyboard.simulate_key(Key::Enter);
    window.redraw_panes();
    auto after_enter_expected = TextPane::StackedHorizontally({
        UNFOCUSED(MenuHighlight.Focused(" A ")),
        FOCUSED(MenuHighlight.Focused(" B ")),
        UNFOCUSED(MenuHighlight.Focused(" C "))
    });
    EXPECT_EQ(window.to_string(), std::string(after_enter_expected));

    keyboard.simulate_key(Key::Escape);
    window.redraw_panes();
    auto after_escape_expected = TextPane::StackedHorizontally({
        UNFOCUSED(MenuHighlight.Focused(" A ")),
        HOVERING_OVER(MenuHighlight.Focused(" B ")),
        UNFOCUSED(MenuHighlight.Focused(" C "))
    });
    EXPECT_EQ(window.to_string(), std::string(after_escape_expected));
}

TEST(WindowFocusTest, KeysOnlyForwardedToFocusedPane) {
    auto keyboard = MockKeyboardSource();
    auto menu_left = Termy::Menu(std::vector<Termy::MenuItem>{
        {"L1", ""}, {"L2", ""}, {"L3", ""}
    });
    auto menu_right = Termy::Menu(std::vector<Termy::MenuItem>{
        {"R1", ""}, {"R2", ""}, {"R3", ""}
    });
    auto window = Termy::Window(12, 5, {
        {.component = menu_left, .percentage = 0.5f, .alignment = ComponentAlignment::Center},
        {.component = menu_right, .percentage = 0.5f, .alignment = ComponentAlignment::Center}
    }, keyboard);

    using Items = std::initializer_list<std::string>;
    auto FOCUSED = [](Items items) { return FocusedBorder.ColouredBorder(6, items); };
    auto UNFOCUSED = [](Items items) { return TextBorder::PaddedContent(6, 5, items); };

    keyboard.simulate_key(Key::Right);
    keyboard.simulate_key(Key::Enter);
    keyboard.simulate_key(Key::Down);
    window.redraw_panes();

    auto after_down_expected = TextPane::StackedHorizontally({
        UNFOCUSED({
            MenuHighlight.Focused(" L1 "),
            MenuHighlight.Unfocused(" L2 "),
            MenuHighlight.Unfocused(" L3 ")
        }),
        FOCUSED({
            MenuHighlight.Unfocused(" R1 "),
            MenuHighlight.Focused(" R2 "),
            MenuHighlight.Unfocused(" R3 ")
        })
    });
    EXPECT_EQ(window.to_string(), std::string(after_down_expected));

    auto HOVERING_OVER = [](Items items) { return CandidateBorder.ColouredBorder(6, items); };

    keyboard.simulate_key(Key::Escape);
    keyboard.simulate_key(Key::Down);
    window.redraw_panes();

    auto after_escape_expected = TextPane::StackedHorizontally({
        UNFOCUSED({
            MenuHighlight.Focused(" L1 "),
            MenuHighlight.Unfocused(" L2 "),
            MenuHighlight.Unfocused(" L3 ")
        }),
        HOVERING_OVER({
            MenuHighlight.Unfocused(" R1 "),
            MenuHighlight.Focused(" R2 "),
            MenuHighlight.Unfocused(" R3 ")
        })
    });
    EXPECT_EQ(window.to_string(), std::string(after_escape_expected));
}
