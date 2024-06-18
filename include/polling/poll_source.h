#pragma once

#include <vector>
#include <chrono>

#include "types.h"

namespace Async {
    // IPollSource is a simple interface that allows for the implementation of a poll source
    // poll sources are objects that must be checked periodically for new work and to drive the completion
    // of any asynchronous tasks
    class IPollSource {
        public:
            virtual auto poll() -> std::vector<Async::job> = 0;
            virtual auto poll_frequency() -> std::chrono::milliseconds = 0;
    };   
}