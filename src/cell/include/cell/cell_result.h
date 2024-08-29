#pragma once

#include <variant>

namespace Cell {
    template <typename T, typename Err>
    using Result = std::variant<T, Err>;

    template <typename T, typename Err, typename VisitT, typename VisitErr>
    auto visit_result(Result<T, Err> result, VisitT visit_t, VisitErr visit_err) -> void {
        if (std::holds_alternative<T>(result)) {
            visit_t(std::get<T>(result));
        } else {
            visit_err(std::get<Err>(result));
        }
    }

    template <typename T, typename Err, typename TFunc, typename ErrFunc>
    auto map_result(Result<T, Err> result, TFunc t_func, ErrFunc err_func) -> decltype(err_func(std::get<Err>(result))) {
        if (std::holds_alternative<T>(result)) {
            return t_func(std::get<T>(result));
        }

        return err_func(std::get<Err>(result));
    }
}