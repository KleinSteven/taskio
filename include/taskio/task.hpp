#pragma once

#include <cassert>
#include <concepts>
#include <coroutine>
#include <exception>

#include <taskio/concept/awaitable.hpp>
#include <taskio/concept/future.hpp>
#include <taskio/concept/promise.hpp>

namespace taskio {

template<typename T>
class task;

namespace detail {

    template<typename T>
    class task_promise_base;

    /**
     * @brief When task<> final, resume its parent_coroutine
     */
    template<typename T>
    struct task_final_awaiter {
        static constexpr bool await_ready() noexcept { return false; }

        template<std::derived_from<task_promise_base<T>> Promise>
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<Promise> current) noexcept {
            return current.promise().parent_coro;
        }

        // Won't be resumed anyway
        constexpr void await_resume() const noexcept {}
    };

    /**
     * @brief When task<> final, resume its parent_coroutine
     */
    template<>
    struct task_final_awaiter<void> {
        static constexpr bool await_ready() noexcept { return false; }

        // Note: the return handle's template parameter shouldn't be Promise
        template<std::derived_from<task_promise_base<void>> Promise>
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<Promise> current) noexcept {
            auto &promise = current.promise();

            std::coroutine_handle<> continuation = promise.parent_coro;

            if (promise.is_detached_flag == Promise::is_detached) {
                current.destroy();
            }

            return continuation;
        }

        // Won't be resumed anyway
        constexpr void await_resume() const noexcept {}
    };

    /**
     * @brief The base class for the promise_type of the task
     */
    template<typename T>
    struct task_promise_base {
        friend struct task_final_awaiter<T>;

        task_promise_base() noexcept = default;

        inline constexpr std::suspend_always initial_suspend() noexcept {
            return {};
        }

        inline constexpr task_final_awaiter<T> final_suspend() noexcept {
            return {};
        }

        inline void set_parent(std::coroutine_handle<> continuation) noexcept {
            parent_coro = continuation;
        }

        task_promise_base(const task_promise_base &) = delete;
        task_promise_base(task_promise_base &&) = delete;
        task_promise_base &operator=(const task_promise_base &) = delete;
        task_promise_base &operator=(task_promise_base &&) = delete;

      private:
        std::coroutine_handle<> parent_coro{std::noop_coroutine()};
    };

    /**
     * @brief The promise_type of the task that return a value
     * @tparam T task' s return value
     */
    template<typename T>
    struct task_promise final : public task_promise_base<T> {
        task_promise() noexcept : state(value_state::mono) {}

        // union types are not automatically destroyed
        ~task_promise() {
            switch (state) {
                [[likely]] case value_state::value:
                    value.~T();
                    break;
                case value_state::exception:
                    exception_ptr.~exception_ptr();
                    break;
                default:
                    break;
            }
        }

        task<T> get_return_object() noexcept;

        void unhandled_exception() noexcept {
            exception_ptr = std::current_exception();
            state = value_state::exception;
        }

        template<typename Value>
            requires std::convertible_to<Value &&, T>
        void return_value(Value &&result
        ) noexcept(std::is_nothrow_constructible_v<T, Value &&>) {
            std::construct_at(
                std::addressof(value), std::forward<Value>(result)
            );
            state = value_state::value;
        }

        // get the lvalue ref
        T &result() & {
            if (state == value_state::exception) [[unlikely]] {
                std::rethrow_exception(exception_ptr);
            }
            assert(state == value_state::value);
            return value;
        }

        // get the prvalue
        T &&result() && {
            if (state == value_state::exception) [[unlikely]] {
                std::rethrow_exception(exception_ptr);
            }
            assert(state == value_state::value);
            return std::move(value);
        }

      private:
        union {
            T value;
            std::exception_ptr exception_ptr;
        };
        enum class value_state : uint8_t { mono, value, exception } state;
    };

    /**
     * @brief The promise_type of the task that no return value
     */
    template<>
    struct task_promise<void> final : public task_promise_base<void> {
        friend struct task_final_awaiter<void>;
        friend class task<void>;

        task_promise() noexcept : is_detached_flag(0){};

        ~task_promise() noexcept {
            if (is_detached_flag != is_detached) {
                exception_ptr.~exception_ptr();
            }
        }

        task<void> get_return_object() noexcept;

        constexpr void return_void() noexcept {}

        void unhandled_exception() {
            if (is_detached_flag == is_detached) {
                std::rethrow_exception(std::current_exception());
            } else {
                exception_ptr = std::current_exception();
            }
        }

        void result() const {
            if (exception_ptr) [[unlikely]] {
                std::rethrow_exception(exception_ptr);
            }
        }

      private:
        inline static constexpr uintptr_t is_detached = -1ULL;

        union {
            uintptr_t is_detached_flag; // set to `is_detached` if is detached.
            std::exception_ptr exception_ptr;
        };
    };

    /**
     * @brief The promise_type of the task that return a lvalue ref
     * @param T the type after remove ref for the lvalue ref
     */
    template<typename T>
    struct task_promise<T &> final : public task_promise_base<T &> {
        task_promise() noexcept = default;

        task<T &> get_return_object() noexcept;

        void unhandled_exception() noexcept {
            exception_ptr = std::current_exception();
        }

        void return_value(T &result) noexcept {
            value = std::addressof(result);
        }

        T &result() {
            if (exception_ptr) [[unlikely]] {
                std::rethrow_exception(exception_ptr);
            }
            return *value;
        }

      private:
        T *value;
        std::exception_ptr exception_ptr;
    };

} // namespace detail

/**
 * @brief The return type of a coroutine and the coroutine is lazy start
 * @tparam T the coroutine's result type, default void
 */
template<typename T = void>
struct task {
    using promise_type = detail::task_promise<T>;

  private:
    struct awaiter_base {
        // The external coroutine handle
        std::coroutine_handle<promise_type> handle;

        explicit awaiter_base(std::coroutine_handle<promise_type> current
        ) noexcept
            : handle(current) {}

        bool await_ready() { return !handle || handle.done(); }

        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<> awaiting) {
            handle.promise().set_parent(awaiting);
            return handle;
        }
    };

  public:
    task() = default;

    explicit task(std::coroutine_handle<promise_type> handle) noexcept
        : handle(handle) {}

    task(const task &) = delete;
    task &operator=(const task &) = delete;

    task(task &&other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    task &operator=(task &&other) noexcept {
        if (this != std::addressof(other)) [[likely]] {
            if (handle) {
                handle.destroy();
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~task() {
        if (handle) {
            handle.destroy();
        }
    }

    void detach() noexcept {
        if constexpr (std::is_void_v<T>) {
            handle.promise().is_detached_flag = promise_type::is_detached;
        }
        handle = nullptr;
    }

    std::coroutine_handle<promise_type> get_handle() noexcept { return handle; }

    /**
     * @brief Overload the co_await operator to get the awaiter
     */
    auto operator co_await() const & noexcept {
        struct awaiter : awaiter_base {
            using awaiter_base::awaiter_base;

            decltype(auto) await_resume() {
                return this->handle.promise().result();
            }
        };

        return awaiter{handle};
    }

    /**
     * @brief Overload the co_await operator to get the awaiter
     */
    auto operator co_await() const && noexcept {
        struct awaiter : awaiter_base {
            using awaiter_base::awaiter_base;

            decltype(auto) await_resume() {
                return std::move(this->handle.promise()).result();
            }
        };

        return awaiter{handle};
    }

  private:
    std::coroutine_handle<promise_type> handle;
};

namespace detail {
    template<typename T>
    inline task<T> task_promise<T>::get_return_object() noexcept {
        return task<T>{
            std::coroutine_handle<task_promise>::from_promise(*this)};
    }

    inline task<void> task_promise<void>::get_return_object() noexcept {
        return task<void>{
            std::coroutine_handle<task_promise>::from_promise(*this)};
    }

    template<typename T>
    inline task<T &> task_promise<T &>::get_return_object() noexcept {
        return task<T &>{
            std::coroutine_handle<task_promise>::from_promise(*this)};
    }

} // namespace detail

// check
//static_assert(concepts::Future<task<int>>);
//static_assert(concepts::Future<task<void>>);
//
//static_assert(concepts::Awaitable<
//              detail::task_final_awaiter<int>,
//              detail::task_promise<int>>);
//
//static_assert(concepts::Awaitable<
//              detail::task_final_awaiter<void>,
//              detail::task_promise<void>>);

} // namespace taskio
