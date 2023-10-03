#pragma once

#include <concepts>

#include <taskio/concept/awaitable.hpp>

namespace taskio::concepts {
    template<typename Fut>
    concept Future = Awaitable<Fut> && requires(Fut fut) {
        requires std::move_constructible<Fut>;
        typename std::remove_cvref_t<Fut>::promise_type;
    };
}
