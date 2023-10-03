#pragma once

#include <atomic>
#include <type_traits>

namespace taskio {

/**
 * cast a trivially_copyable type to a const std::atomic type
 * @tparam T must be a trivially_copyable type
 * @param value the value of a trivially_copyable type
 * @return the value of const std::atomic type
 */
template<typename T>
    requires std::is_trivially_copyable_v<T>
inline const std::atomic<T> &as_atomic(const T &value) noexcept {
    return *reinterpret_cast<const std::atomic<T> *>(std::addressof(value));
}

}
