#pragma once

#include <cstdint>

namespace taskio {

namespace config {
    enum class level : uint8_t { log, debug, warning, err, none};

    inline constexpr level log_level = level::log;
    // inline constexpr level log_level = level::debug;
    // inline constexpr level log_level = level::warning;
    // inline constexpr level log_level = level::err;
    // inline constexpr level log_level = level::none;

    inline constexpr bool is_log_level = log_level <= level::log;
    inline constexpr bool is_debug_level = log_level <= level::debug;
    inline constexpr bool is_warning_level = log_level <= level::warning;
    inline constexpr bool is_err_level = log_level <= level::err;

    using cur_t = uint16_t;
    inline constexpr cur_t spsc_capacity = 16384;

    using ctx_id_t = uint16_t;

    inline constexpr std::size_t cache_line_size = 64;

}

}
