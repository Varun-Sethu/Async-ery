#include "keyboard_poll_source.h"

#include <algorithm>
#include <array>
#include <unistd.h>
#include <fcntl.h>

using std::chrono_literals::operator""ms;

namespace Termy {

KeyboardPollSource::KeyboardPollSource()
    : term_()
    , original_term_()
{
    tcgetattr(STDIN_FILENO, &original_term_);
    term_ = original_term_;
    term_.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term_);

    auto orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, orig_flags | O_NONBLOCK);
}

KeyboardPollSource::~KeyboardPollSource()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term_);
    auto orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, orig_flags & ~O_NONBLOCK);
}

auto KeyboardPollSource::add_listener(IKeyboardListener& listener) -> void {
    listeners_.push_back(&listener);
}

auto KeyboardPollSource::remove_listener(IKeyboardListener& listener) -> void {
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), &listener),
        listeners_.end()
    );
}

auto KeyboardPollSource::poll_frequency() -> std::chrono::milliseconds
{
    return 5ms;
}

auto KeyboardPollSource::poll() -> std::vector<Scheduler::Job>
{
    auto byte_buffer = std::array<char, 3>();
    auto num_bytes = read(STDIN_FILENO, &byte_buffer[0], sizeof(char) * byte_buffer.size());

    if (num_bytes == 1) {
        if (byte_buffer[0] == '\n' || byte_buffer[0] == '\r') {
            return create_listener_notification_jobs(Key::Enter);
        }
        if (byte_buffer[0] == '\033') {
            return create_listener_notification_jobs(Key::Escape);
        }
    }

    if (num_bytes != 3) {
        return {};
    }

    if (byte_buffer[0] == '\033' && byte_buffer[1] == '[') {
        switch (byte_buffer[2]) {
            case 'A': return create_listener_notification_jobs(Key::Up);
            case 'B': return create_listener_notification_jobs(Key::Down);
            case 'C': return create_listener_notification_jobs(Key::Right);
            case 'D': return create_listener_notification_jobs(Key::Left);
            default: break;
        }
    }

    return {};
}

auto KeyboardPollSource::create_listener_notification_jobs(Key key) -> std::vector<Scheduler::Job> {
    auto jobs = std::vector<Scheduler::Job>();
    for (auto* listener : listeners_) {
        auto notify = [listener, key](auto) { listener->on_key_press(key); };

        // std::function uses Small Buffer Optimization (SBO) to avoid heap allocation
        // for small callables. The threshold is implementation-specific:
        // libstdc++ (GCC): 16 bytes, libc++ (Clang): 24 bytes, MSVC: ~32 bytes.
        // We use 16 as the conservative minimum across all major implementations.
        static_assert(sizeof(notify) <= 16, "Lambda exceeds std::function SBO threshold");

        jobs.emplace_back(std::move(notify));
    }
    return jobs;
}

}
