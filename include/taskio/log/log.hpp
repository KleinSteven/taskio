#pragma once

#include <format>

#include <taskio/config.hpp>

namespace taskio {

namespace detail {
    template<typename... Args>
    void log(std::string_view fmt, Args &&...args) {
        auto fmt_args{std::make_format_args(args...)};
        std::string output{std::vformat(fmt, fmt_args)};
        std::fputs(output.c_str(), stdout);
    }

    template<typename... Args>
    void err(std::string_view fmt, Args &&...args) {
        auto fmt_args{std::make_format_args(args...)};
        std::string output{std::vformat(fmt, fmt_args)};
        std::fputs(output.c_str(), stderr);
    }
} // namespace detail

namespace log {
    template<typename... Args>
    inline void log(std::string_view fmt, Args &&...args) {
        if constexpr (config::is_log_level) {
            detail::log(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    inline void debug(std::string_view fmt, Args &&...args) {
        if constexpr (config::is_debug_level) {
            detail::log(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    inline void warn(std::string_view fmt, Args &&...args) {
        if constexpr (config::is_warning_level) {
            detail::err(fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    inline void err(std::string_view fmt, Args &&...args) {
        if constexpr (config::is_err_level) {
            detail::err(fmt, std::forward<Args>(args)...);
        }
    }
} // namespace log

} // namespace taskio
