#pragma once

#include <taskio/concept/awaitable.hpp>
#include <taskio/concept/future.hpp>

namespace taskio::concepts {
template<typename P, typename Result = void>
concept Promise = requires(P p) {
    { p.get_return_object() } -> Future;
    { p.initial_suspend() } -> Awaitable;
    { p.final_suspend() } noexcept -> Awaitable;
    p.unhandled_exception();
    requires(
        requires(Result v) { p.return_value(v); } || requires { p.return_void(); }
    );
};
}
