#pragma once

#include <mutex>
#include <thread>

#include <taskio/detail/io_context_info.hpp>
#include <taskio/detail/thread_info.hpp>
#include <taskio/detail/worker_meta.hpp>
#include <taskio/task.hpp>

namespace taskio {

struct io_context {
    explicit io_context() noexcept {
        auto &meta = detail::io_context_info;
        std::lock_guard lock(meta.mtx);
        this->id = meta.create_count++;
    }

    ~io_context() = default;

    io_context(const io_context &) = delete;
    io_context(io_context &&) = delete;
    io_context &operator=(const io_context &) = delete;
    io_context &operator=(io_context &&) = delete;

    void spawn(task<void> &&task) noexcept;

    void start();

    void join();

  private:
    void init() noexcept;

    void deinit() noexcept;

    void run();

    void get_process() noexcept;

  private:

    alignas(config::cache_line_size) detail::worker_meta work;

    std::jthread thread;

    __pid_t tid;

    config::ctx_id_t id;
    bool stop = false;
};

} // namespace taskio
