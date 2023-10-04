#pragma once

#include <condition_variable>
#include <mutex>

#include <taskio/config.hpp>

namespace taskio::detail {

struct io_context_info {
    std::mutex mtx;
    std::condition_variable cv;
    config::ctx_id_t create_count{};
    config::ctx_id_t ready_count{};
};

inline io_context_info io_context_info;

} // namespace taskio::detail
