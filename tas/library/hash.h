#pragma once

#include <cstdint>

namespace hash {
  template <typename T> constexpr inline uint32_t hash_crc(T* data, uint32_t value = 0) {
    return !(*data) ? value : hash_crc(&data[1], value * 31 + (uint32_t)data[0]);
  }
} // namespace hash

#define HASH(hash_value) std::integral_constant<uint32_t, hash::hash_crc(hash_value)>::value
