#pragma once

#include <vector>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>

#include "scheduler/poll_source.h"
#include "termios.h"

#include "key.h"

using std::chrono_literals::operator""ms;

namespace Termy {
    using KeyListener = std::function<void(Key)>;

    class KeyboardPollSource : public Scheduler::IPollSource {
    public:
        KeyboardPollSource() : term()
        {
            //  Configure the terminal window accordingly
            tcgetattr(STDIN_FILENO, &term);
            term.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &term);

            // Set stdin to non-blocking
            auto orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, orig_flags | O_NONBLOCK);
        }

        ~KeyboardPollSource()
        {
            // Restore terminal settings
            tcsetattr(STDIN_FILENO, TCSANOW, &term);
            auto orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, orig_flags & ~O_NONBLOCK);
        }

        auto add_listener(const KeyListener& listener) -> void
        {
            listeners.push_back(listener);
        }

        auto poll_frequency() -> std::chrono::milliseconds override { return 5ms; }

        auto poll() -> std::vector<Scheduler::Job> override
        {
            auto byte_buffer = std::array<char, 3>();
            auto num_bytes = read(STDIN_FILENO, &byte_buffer[0], sizeof(char) * byte_buffer.size());
            if (num_bytes == 1) {
                // Handle Enter key (\n or \r)
                if (byte_buffer[0] == '\n' || byte_buffer[0] == '\r') {
                    return create_listener_notification_jobs(Key::Enter);
                }
            }
            if (num_bytes != 3) {
                // No input is available on stdin at the moment so it is safe to return an empty vector
                return {};
            }

            if (num_bytes == 3) {
                if (byte_buffer[0] == '\033' && byte_buffer[1] == '[') {
                    switch (byte_buffer[2]) {
                        case 'A': return create_listener_notification_jobs(Key::Up);
                        case 'B': return create_listener_notification_jobs(Key::Down);
                        case 'C': return create_listener_notification_jobs(Key::Right);
                        case 'D': return create_listener_notification_jobs(Key::Left);
                        default: break;
                    }
                }
            }

            return {};
        }

    private:
        auto create_listener_notification_jobs(Key key) -> std::vector<Scheduler::Job>
        {
            auto jobs = std::vector<Scheduler::Job>();
            for (const auto& listener : listeners) {
                jobs.emplace_back([listener, key](auto) { listener(key); });
            }
    
            return jobs;
        }
    
        termios term;
        std::vector<KeyListener> listeners;
    };
}