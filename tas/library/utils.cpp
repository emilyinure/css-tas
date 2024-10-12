#include "utils.h"

#include <utf8/unchecked.h>

namespace utils {
  std::string str_utf16_to_8(std::wstring_view str) noexcept {
    std::string res{};
    utf8::unchecked::utf16to8(str.begin(), str.end(), std::back_inserter(res));

    return res;
  }

  std::wstring str_utf8_to_16(std::string_view str) noexcept {
    std::wstring res{};
    utf8::unchecked::utf8to16(str.begin(), str.end(), std::back_inserter(res));

    return res;
  }
} // namespace utils