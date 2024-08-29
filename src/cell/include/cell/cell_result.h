#pragma once

#include <variant>
#include <concepts>

namespace Cell {
    template <typename T, typename Err>
    using Result = std::variant<T, Err>;

    template <typename T, typename Err, std::invocable<T> IfT, std::invocable<Err> IfErr>
    auto visit_result(Result<T, Err> result, IfT visit_t, IfErr visit_err) -> void {
        if (std::holds_alternative<T>(result)) {
            visit_t(std::get<T>(result));
        } else {
            visit_err(std::get<Err>(result));
        }
    }

    template <typename T, typename Err, std::invocable<T> IfT, std::invocable<Err> IfErr>
    auto map_result(Result<T, Err> result, IfT t_func, IfErr err_func) -> decltype(auto) {
        if (std::holds_alternative<T>(result)) {
            return t_func(std::get<T>(result));
        }

        return err_func(std::get<Err>(result));
    }
}