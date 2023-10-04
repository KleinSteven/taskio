#include <taskio/detail/thread_info.hpp>
#include <taskio/detail/worker_meta.hpp>
#include <taskio/log/log.hpp>

using namespace taskio::log;

namespace taskio::detail {

thread_local thread_info this_thread;

void worker_meta::init() noexcept {
    this_thread.worker = this;
}

void worker_meta::deinit() noexcept {
    this_thread.worker = nullptr;
}

std::coroutine_handle<> worker_meta::schedule() noexcept {
    return ready_task.fetch_task();
}

void worker_meta::work_once() noexcept {
    auto coro = this->schedule();
    coro.resume();
}

void worker_meta::post_task(std::coroutine_handle<> handle) noexcept {
    ready_task.post_task(handle);
}

}
