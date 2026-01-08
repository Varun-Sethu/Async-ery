#pragma once

#include <stdexcept>

#include "interface/io/io_poll_source_intf.h"

namespace Mocks {
    class MockIOPollSource : public IO::IPollSource {
    public:
        auto poll_frequency() -> std::chrono::milliseconds override {
            return std::chrono::milliseconds(1);
        }

        auto poll() -> std::vector<Scheduler::Job> override {
            return {};
        }

        auto queue_read(FILE*, IO::ReadRequest, const Callback&) -> void override {
            throw std::runtime_error("MockIOPollSource::queue_read not implemented");
        }
    };
}
