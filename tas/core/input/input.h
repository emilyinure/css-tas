#pragma once

#include <safetyhook.hpp>
#include <glm/vec2.hpp>

#include <cstdint>
#include <array>
#include <atomic>
#include <Windows.h>

enum class InputState : uint8_t { Up, Down };

class input_t {
public:
  input_t() = default;

  bool key_down(uint8_t vk) const noexcept;
  bool key_pressed(uint8_t vk) const noexcept;

  void on_menu_open() noexcept;
  void on_menu_close() noexcept;

  void initialize() noexcept;
  void detach() noexcept;
  void pre_check() noexcept;
  void post_check() noexcept;

  SafetyHookInline SetCursorPos_hook{};
  SafetyHookInline GetCursorPos_hook{};
  SafetyHookInline ShowCursor_hook{};
  SafetyHookInline SetCursor_hook{};
  SafetyHookInline GetCursor_hook{};
  SafetyHookInline GetCursorInfo_hook{};
  SafetyHookInline ClipCursor_hook{};
  SafetyHookInline GetClipCursor_hook{};

  POINT        last_cursor_pos{};
  std::int32_t show_cursor_count{};
  HCURSOR      last_cursor{};
  RECT         last_cursor_clip{};
  WNDPROC      orig_wndproc{};

  std::array<InputState, 256> keys      = {InputState::Up};
  std::array<InputState, 256> last_keys = {InputState::Up};

  std::recursive_mutex imgui_mutex{};
};
