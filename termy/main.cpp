#include <iostream>
#include <atomic>
#include <csignal>
#include <thread>

#include "async_lib/task_factory.h"

#include "include/keyboard_poll_source.h"
#include "include/frame.h"
#include "include/menu.h"
#include "include/text_box.h"
#include "include/window.h"

using namespace std::chrono_literals;

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

auto main() -> int {
    std::signal(SIGINT, signal_handler);

    auto keyboard_poll_source = std::make_shared<Termy::KeyboardPollSource>();
    auto task_factory = Async::TaskFactory(1, { keyboard_poll_source });
    auto timer_source = task_factory.timer_source();
    auto colors = Termy::MenuColors{
        .focused_fg = Termy::Color::Black,
        .focused_bg = Termy::Color::Cyan,
        .unfocused_fg = Termy::Color::White,
        .unfocused_bg = Termy::Color::Default
    };

    auto menu_items = std::vector<Termy::MenuItem>{
        {"Option 1", "Option 1: Chicken nuggies?"},
        {"Option 2", "Option 2: Tomatoes and potatoes, whats the diff?"},
        {"Option 3", "Option 3: The least epic option"},
        {"Exit", "Please dont exit the application :c"}
    };

    auto menu_two_items = std::vector<Termy::MenuItem>{
        {"Tomato A", "Option A: Chicken nuggies?"},
        {"Tomato B", "Option B: Tomatoes and potatoes, whats the diff?"},
        {"Tomato C", "Option C: The least epic option"},
        {"Exit", "Please dont exit the application :c"}
    };

    auto menu_two = Termy::Menu(menu_two_items, colors);
    auto menu = Termy::Menu(menu_items, colors);

    auto text_box_colors = Termy::TextBoxColors{
        .text_fg = Termy::Color::White,
        .text_bg = Termy::Color::Default,
        .cursor_fg = Termy::Color::Black,
        .cursor_bg = Termy::Color::Cyan
    };
    auto text_box = Termy::TextBox(
        "helloooooooooooooooooooooooo\n"
        "oooooooooooooooooooooo\n"
        "ooo",
        text_box_colors
    );

    auto window = Termy::Window(80, 12, {
        {.component = menu, .percentage = 0.25f, .alignment = Termy::ComponentAlignment::Center},
        {.component = text_box, .percentage = 0.5f, .alignment = Termy::ComponentAlignment::Left},
        {.component = menu_two, .percentage = 0.25f, .alignment = Termy::ComponentAlignment::Center}
    }, *keyboard_poll_source);

    std::cout << "\033[2J";
    std::cout << "\033[?25l";

    timer_source.periodic(33ms).for_each([&window](auto) {
        std::cout << "\033[1;1H";

        window.clear();
        window.redraw_panes();
        std::cout << window.to_string();
        std::cout.flush();
    });

    while (running) {
        std::this_thread::sleep_for(100ms);
    }

    std::cout << "\033[?25h";
    std::cout << "\033[0m";
    std::cout << "\033[2J";
    std::cout << "\033[1;1H";
    std::cout.flush();

    return 0;
}
