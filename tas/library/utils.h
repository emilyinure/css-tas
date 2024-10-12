#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace utils {
  template <typename T> T get_virtual_function(void* base, std::uint16_t index) noexcept {
    return (*static_cast<T**>(base))[index];
  }

  // UTF-16 -> UTF-8.
  extern std::string str_utf16_to_8(std::wstring_view str) noexcept;

  // UTF-8 -> UTF-16.
  extern std::wstring str_utf8_to_16(std::string_view str) noexcept;
} // namespace utils
