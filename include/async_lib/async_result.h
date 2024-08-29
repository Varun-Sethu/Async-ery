#pragma once

#include <variant>

namespace Async {
    enum Error {
        Rejected
    };

    template <typename T>
    using Result = std::variant<T, Error>;
};