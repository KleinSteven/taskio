#pragma once

#include <concepts>
#include <coroutine>
#include <utility>

namespace taskio {

namespace detail {
    template<typename Awaitable>
    struct GetAwaiter : std::type_identity<Awaitable> {};

    template<typename Awaitable>
        requires requires(Awaitable &&awaitable) {
            std::forward<Awaitable>(awaitable).operator co_await();
        }
    struct GetAwaiter<Awaitable>
        : std::type_identity<
              decltype(std::declval<Awaitable>().operator co_await())> {};

    template<typename Awaitable>
        requires requires(Awaitable &&awaitable) {
            operator co_await(std::forward<Awaitable>(awaitable));
            requires !(requires {
                std::forward<Awaitable>(awaitable).operator co_await();
            });
        }
    struct GetAwaiter<Awaitable>
        : std::type_identity<decltype(operator co_await(std::declval<Awaitable>(
          )))> {};

    template<typename Awaitable>
    using GetAwaiter_t = typename GetAwaiter<Awaitable>::type;

} // namespace detail

namespace concepts {
    template<typename A, typename T = void>
    concept Awaitable = requires {
        typename detail::GetAwaiter_t<A>;
        requires requires(
            detail::GetAwaiter_t<A> awaiter, std::coroutine_handle<T> handle
        ) {
            { awaiter.await_ready() } -> std::convertible_to<bool>;
            awaiter.await_suspend(handle);
            awaiter.await_resume();
        };
    };
}

template<concepts::Awaitable A>
using AwaitResult =
    decltype(std::declval<detail::GetAwaiter_t<A>>().await_resume());

// check concepts
static_assert(concepts::Awaitable<std::suspend_always>);
static_assert(concepts::Awaitable<std::suspend_never>);

} // namespace taskio
