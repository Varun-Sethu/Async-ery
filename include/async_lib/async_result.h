#pragma once

#include <variant>

namespace Async {
    enum Error {
        Rejected,
        IOError,
    };

    auto inline error_to_string(Error error) -> const char* {
        switch (error) {
            case Rejected:
                return "Rejected";
            case IOError:
                return "IOError";
            default:
                return "Unknown";
        }
    }

    template <typename T>
    using Result = std::variant<T, Error>;
};