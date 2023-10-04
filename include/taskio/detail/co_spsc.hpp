#pragma once

#include <cassert>
#include <array>
#include <coroutine>

#include <taskio/config.hpp>
#include <taskio/detail/safety.hpp>
#include <taskio/util/to_atomic.hpp>

namespace taskio::detail {

/**
 *
 * @tparam T The size type of the spsc
 * @tparam is_thread_safe Whether thread-safe practices are employed.
 * For performance reasons, thread-unsafe practices are used by default
 */
template<
    std::unsigned_integral T = config::cur_t,
    T capacity = config::spsc_capacity,
    bool is_thread_safe = safety::unsafe>
struct spsc {

    inline void post_task(std::coroutine_handle<> handle) noexcept {
        assert(bool(handle) && "handle cannot be empty");
        reap_queue[cursor_.tail()] = handle;
        cursor_.push();
    }

    inline std::coroutine_handle<> fetch_task() noexcept {
        assert(!cursor_.is_empty() && "spsc is empty");
        std::coroutine_handle<> coro = this->reap_queue[cursor_.head()];
        cursor_.pop();
        return coro;
    }

    inline T task_num() noexcept {
        return cursor_.size();
    }

  private:
    template<std::unsigned_integral Type, Type capacity_>
    struct cursor {
        using sz_t = Type;
        sz_t m_head = 0;
        sz_t m_tail = 0;

        inline static constexpr sz_t mask = capacity_ - 1;

        [[nodiscard]]
        inline sz_t head() const noexcept {
            return m_head & mask;
        }

        [[nodiscard]]
        inline sz_t tail() const noexcept {
            return m_tail & mask;
        }

        [[nodiscard]]
        inline bool is_empty() const noexcept {
            return m_head == m_tail;
        }

        [[nodiscard]]
        inline sz_t remain_sz() const noexcept {
            return capacity_ - (m_tail - m_head);
        }

        inline sz_t size() const noexcept { return m_tail - m_head; }

        [[nodiscard]]
        inline bool is_available() const noexcept {
            return bool(capacity_ - (m_tail - m_head));
        }

        [[nodiscard]]
        inline sz_t raw_head() const noexcept {
            return m_head;
        }

        [[nodiscard]]
        inline sz_t raw_tail() const noexcept {
            return m_tail;
        }

        [[nodiscard]]
        inline sz_t load_head() const noexcept {
            if constexpr (is_thread_safe) {
                return as_atomic(m_head).load(std::memory_order_acquire) & mask;
            } else {
                return head();
            }
        }

        [[nodiscard]]
        inline sz_t load_tail() const noexcept {
            if constexpr (is_thread_safe) {
                return as_atomic(m_tail).load(std::memory_order_acquire) & mask;
            } else {
                return tail();
            }
        }

        [[nodiscard]]
        inline sz_t load_raw_head() const noexcept {
            if constexpr (is_thread_safe) {
                return as_atomic(m_head).load(std::memory_order_acquire);
            } else {
                return raw_head();
            }
        }

        [[nodiscard]]
        inline sz_t load_raw_tail() const noexcept {
            if constexpr (is_thread_safe) {
                return as_atomic(m_tail).load(std::memory_order_acquire);
            } else {
                return raw_tail();
            }
        }

        inline void push(sz_t num = 1) noexcept {
            if constexpr (is_thread_safe) {
                as_atomic(m_tail).store(
                    m_tail + num, std::memory_order_release
                );
            } else {
                m_tail += num;
            }
        }

        inline void pop(sz_t num = 1) noexcept {
            if constexpr (is_thread_safe) {
                as_atomic(m_head).store(
                    m_head + num, std::memory_order_release
                );
            } else {
                m_head += num;
            }
        }
    };

    using cur_t = config::cur_t;

    alignas(config::cache_line_size
    ) std::array<std::coroutine_handle<>, capacity> reap_queue;

    cursor<T, capacity> cursor_;
};
} // namespace taskio::detail
