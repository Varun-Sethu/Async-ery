#pragma once

#include <variant>
#include <concepts>

namespace IO {
    enum AIOError {
        Canceled,
        NoExist,
        Unknown,
        InProgress
    };

    template <typename T>
    using AIOResult = std::variant<T, AIOError>;

    template <typename T, std::invocable<T> OnSuccess, std::invocable<AIOError> OnError>
    auto visit_aio_result(const AIOResult<T>& result, OnSuccess on_success, OnError on_error) -> void {
        if (std::holds_alternative<T>(result)) {
            on_success(std::get<T>(result));
        } else {
            on_error(std::get<AIOError>(result));
        }
    }
}