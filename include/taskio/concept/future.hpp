#pragma once

#include <concepts>

#include <taskio/concept/awaitable.hpp>

namespace taskio::concepts {
    template<typename Fut>
    concept Future = Awaitable<Fut> && requires(Fut fut) {
        // It must be constructed through a promise rather than a default
        // construct, and constructs can be moved
        requires !std::default_initializable<Fut>;
        requires std::move_constructible<Fut>;
        typename std::remove_cvref_t<Fut>::promise_type;
        fut.get_result();
    };
}
