#include <iostream>
#include <chrono>
#include <atomic>
#include <csignal>
#include <thread>

#include "async_lib/task_factory.h"

#include "include/keyboard_poll_source.h"
#include "include/menu.h"
#include "include/frame.h"

using namespace std::chrono_literals;

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

int main() {
    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signal_handler);

    auto keyboard_poll_source = std::make_shared<Termy::KeyboardPollSource>();
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 1, { keyboard_poll_source });
    auto timer_source = task_factory.timer_source();

    auto menu_items = std::vector<Termy::MenuItem>{
        {"Option 1", "Option 1: Chicken nuggies?"},
        {"Option 2", "Option 2: Tomatoes and potatoes, whats the diff?"},
        {"Option 3", "Option 3: The least epic option"},
        {"Exit", "Please dont exit the application :c"}
    };

    auto colors = Termy::MenuColors{
        .focused_fg = Termy::Color::Black,
        .focused_bg = Termy::Color::Cyan,
        .unfocused_fg = Termy::Color::White,
        .unfocused_bg = Termy::Color::Default
    };

    auto menu = Termy::Menu(menu_items, *keyboard_poll_source, colors);

    // Clear screen and hide cursor
    std::cout << "\033[2J";
    std::cout << "\033[?25l";

    // Render loop at ~30fps (33ms per frame)
    timer_source.periodic(33ms).for_each([&menu](auto) {
        // Move cursor to top-left
        std::cout << "\033[1;1H";

        auto frame = Termy::Frame();
        menu.render(frame);
        std::cout << frame.to_string();
        std::cout.flush();
    });

    // Keep main alive until Ctrl+C
    while (running) {
        std::this_thread::sleep_for(100ms);
    }

    // Cleanup: show cursor and reset colors
    std::cout << "\033[?25h";
    std::cout << "\033[0m";
    std::cout << "\033[2J";
    std::cout << "\033[1;1H";
    std::cout.flush();

    return 0;
}
