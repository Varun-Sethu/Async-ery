#include <map>

#include "async_lib/task_factory.h"

#include "include/keyboard_poll_source.h"


int main() {
    auto keyboard_poll_source = std::make_shared<Termy::KeyboardPollSource>();
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 1, { keyboard_poll_source });
    auto timer_task_source = task_factory.timer_source();

    auto menu_tooltips = std::map<std::string, std::string>({
        {"Option 1", "Option 1: Chicken nuggies?"},
        {"Option 2", "Option 2: Tomatoes and potatoes, whats the diff?"},
        {"Option 3", "Option 3: The least epic option"},
        {"Exit", "Please dont exit the application :c"}
    });

    return 0;
}
