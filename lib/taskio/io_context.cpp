#include <taskio/io_context.hpp>
#include <taskio/detail/thread_info.hpp>

#include <unistd.h>

namespace taskio {

void io_context::init() noexcept {
    detail::this_thread.ctx_id = this->id;
    detail::this_thread.ctx = this;
    this->work.init();
    this->tid = ::gettid();
}

void io_context::deinit() noexcept {
    detail::this_thread.ctx_id = -1;
    detail::this_thread.ctx = nullptr;
    this->work.deinit();

    auto &meta = detail::io_context_info;
    std::lock_guard lock(meta.mtx);
    meta.create_count--;
    meta.ready_count--;
}

void io_context::start() {
    thread = std::jthread([this]{
        this->init();
        auto &meta = detail::io_context_info;
        {
            std::unique_lock lock(meta.mtx);
            meta.ready_count++;
            if (!meta.cv.wait_for(lock, std::chrono::seconds{1}, [] {
                    return meta.create_count == meta.ready_count;
            })) {
                std::terminate();
            }
        }
        meta.cv.notify_all();

        this->run();
    });
}

void io_context::run() {
    while (!stop) [[likely]] {
        get_process();

        if (work.task_num() == 0) {
            break;
        }
    }

    this->deinit();
}

void io_context::get_process() noexcept {
    for (auto num = work.task_num(); num > 0; num--) {
        work.work_once();
    }
}

void io_context::spawn(task<void> &&task) noexcept {
    auto handle = task.get_handle();
    task.detach();
    work.post_task(handle);
}

}
