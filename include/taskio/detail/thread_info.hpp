#pragma once

#include <taskio/config.hpp>

namespace taskio {

struct io_context;

}

namespace taskio::detail {

struct worker_meta;

struct alignas(config::cache_line_size) thread_info {
    io_context *ctx = nullptr;
    worker_meta *worker = nullptr;

    config::ctx_id_t ctx_id = static_cast<config::ctx_id_t>(-1);
};

extern thread_local thread_info this_thread;

} // namespace taskio::detail
