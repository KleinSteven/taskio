#pragma once

#include <taskio/detail/co_spsc.hpp>

namespace taskio::detail {

struct worker_meta {
    void init() noexcept;

    void deinit() noexcept;

    std::coroutine_handle<> schedule() noexcept;

    void work_once() noexcept;

    void post_task(std::coroutine_handle<> handle) noexcept;

    auto task_num() noexcept { return ready_task.task_num(); }

    worker_meta() = default;

  private:
    // the number of I/O tasks running in the io_uring
    uint32_t requests_to_reap = 0;
    spsc<config::cur_t, config::spsc_capacity, safety::unsafe> ready_task;
};

} // namespace taskio::detail
