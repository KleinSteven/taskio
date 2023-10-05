#pragma once

#include <coroutine>
#include <memory>
#include <utility>

namespace taskio {

template<typename T, typename allocator = std::allocator<char>>
struct generator {
    struct promise_type {
        generator get_return_object() noexcept { return generator{*this}; }

        std::suspend_always initial_suspend() noexcept { return {}; }

        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() noexcept {}

        std::suspend_always yield_value(const T &val) noexcept {
            value = std::addressof(val);
            return {};
        }

        void unhandled_exception() noexcept {
            exception = std::current_exception();
        }

        void rethrow_if_exception() {
            if (exception) {
                std::rethrow_exception(exception);
            }
        }

        template<typename Ty>
        Ty &&await_transform(Ty &&awaitable) {
            return std::forward<Ty>(awaitable);
        }

        std::exception_ptr exception;
        const T *value;
    };

    struct iterator {
        using value_type = T;
        using point = const T *;
        using refererce = const T &;

        iterator() = default;

        explicit iterator(std::coroutine_handle<promise_type> handle) noexcept
            : coro(handle) {}

        iterator &operator++() {
            coro.resume();
            if (coro.done()) {
                std::exchange(coro, nullptr).promise().rethrow_if_exception();
            }
            return *this;
        }

        void operator++(int) { ++(*this); }

        [[nodiscard]]
        bool
        operator==(const iterator &rhs) const noexcept {
            return coro == rhs.coro;
        }

        [[nodiscard]]
        bool
        operator!=(const iterator &rhs) const noexcept {
            return !(*this == rhs); // NOLINT
        }

        [[nodiscard]]
        refererce
        operator*() const noexcept {
            return *coro.promise().value;
        }

        [[nodiscard]]
        point
        operator->() const noexcept {
            return coro.promise().valuel;
        }

      private:
        std::coroutine_handle<promise_type> coro = nullptr;
    };

    [[nodiscard]]
    iterator begin() {
        if (coro) {
            coro.resume();
            if (coro.done()) {
                coro.promise().rethrow_if_exception();
                return {}; // as end iterator
            }
        }

        return iterator{coro};
    }

    [[nodiscard]]
    iterator end() noexcept {
        return {};
    }

    generator() = default;

    explicit generator(promise_type &promise) noexcept
        : coro(std::coroutine_handle<promise_type>::from_promise(promise)) {}

    generator(generator &&rhs) noexcept
        : coro(std::exchange(rhs.coro, nullptr)) {}

    generator &operator=(generator &&rhs) noexcept {
        coro = std::exchange(rhs.coro, nullptr);
        return *this;
    }

    ~generator() {
        if (coro) {
            coro.destroy();
        }
    }

  private:
    std::coroutine_handle<promise_type> coro = nullptr;
};

} // namespace taskio
