#include <gtest/gtest.h>

#include "window.h"
#include "menu.h"
#include "color.h"
#include "mocks/mock_keyboard_source.h"

using Termy::Color;
using Termy::ColouredString;
using Termy::MockKeyboardSource;

static auto Focused(const std::string& text) -> std::string {
    return ColouredString(text).foreground(Color::Black).background(Color::White).ansi();
}

static auto Unfocused(const std::string& text) -> std::string {
    return ColouredString(text).foreground(Color::White).background(Color::Default).ansi();
}

TEST(WindowTest, RendersSinglePaneFullWidth) {
    auto items = std::vector<Termy::MenuItem>{{"Item", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    auto window = Termy::Window(6, 1, {
        {.component = menu, .percentage = 1.0f}
    });

    window.redraw_panes();
    auto output = window.to_string();

    auto expected = Focused(" Item ") + "\n";
    EXPECT_EQ(output, expected);
}

TEST(WindowTest, RendersTwoPanesSideBySide) {
    auto items_left = std::vector<Termy::MenuItem>{{"L", ""}};
    auto items_right = std::vector<Termy::MenuItem>{{"R", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu_left = Termy::Menu(items_left, keyboard);
    auto menu_right = Termy::Menu(items_right, keyboard);

    auto window = Termy::Window(6, 1, {
        {.component = menu_left, .percentage = 0.5f},
        {.component = menu_right, .percentage = 0.5f}
    });

    window.redraw_panes();
    auto output = window.to_string();

    auto expected = Focused(" L  R ") + "\n";
    EXPECT_EQ(output, expected);
}

TEST(WindowTest, ClearsBufferBetweenRenders) {
    auto items = std::vector<Termy::MenuItem>{{"A", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    auto window = Termy::Window(3, 1, {
        {.component = menu, .percentage = 1.0f}
    });

    window.redraw_panes();
    window.clear();
    window.redraw_panes();

    auto output = window.to_string();
    auto expected = Focused(" A ") + "\n";
    EXPECT_EQ(output, expected);
}

TEST(WindowTest, CalculatesPercentageWidthsCorrectly) {
    auto items1 = std::vector<Termy::MenuItem>{{"X", ""}};
    auto items2 = std::vector<Termy::MenuItem>{{"Y", ""}};
    auto items3 = std::vector<Termy::MenuItem>{{"Z", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu1 = Termy::Menu(items1, keyboard);
    auto menu2 = Termy::Menu(items2, keyboard);
    auto menu3 = Termy::Menu(items3, keyboard);

    auto window = Termy::Window(9, 1, {
        {.component = menu1, .percentage = 0.33f},
        {.component = menu2, .percentage = 0.33f},
        {.component = menu3, .percentage = 0.34f}
    });

    window.redraw_panes();
    auto output = window.to_string();

    auto expected = Focused(" X Y Z ") + "  \n";
    EXPECT_EQ(output, expected);
}

TEST(WindowTest, CentersContentInWiderPane) {
    auto items = std::vector<Termy::MenuItem>{{"Hi", ""}};
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    auto window = Termy::Window(8, 1, {
        {.component = menu, .percentage = 1.0f}
    });

    window.redraw_panes();
    auto output = window.to_string();

    auto expected = "  " + Focused(" Hi ") + "  \n";
    EXPECT_EQ(output, expected);
}

TEST(WindowTest, MultipleRowsRenderedCorrectly) {
    auto items = std::vector<Termy::MenuItem>{
        {"One", ""},
        {"Two", ""}
    };
    auto keyboard = MockKeyboardSource();
    auto menu = Termy::Menu(items, keyboard);

    auto window = Termy::Window(5, 2, {
        {.component = menu, .percentage = 1.0f}
    });

    window.redraw_panes();
    auto output = window.to_string();

    auto expected =
        Focused(" One ") + "\n" +
        Unfocused(" Two ") + "\n";
    EXPECT_EQ(output, expected);
}
