#include "input.h"
#include "cheat.h"
#include "core/menu/menu.h"

#include <cstdint>
#include <ostream>

extern long ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

BOOL WINAPI hooked_SetCursorPos(std::int32_t x, std::int32_t y) noexcept {
  if (!menu::open) {
    return cheat::input.SetCursorPos_hook.stdcall<BOOL>(x, y);
  }

  cheat::input.last_cursor_pos = {x, y};

  return TRUE;
}

BOOL WINAPI hooked_GetCursorPos(LPPOINT point) noexcept {
  if (!menu::open) {
    return cheat::input.GetCursorPos_hook.stdcall<BOOL>(point);
  }

  if (point != nullptr) {
    *point = cheat::input.last_cursor_pos;
  }

  return TRUE;
}

std::int32_t WINAPI hooked_ShowCursor(BOOL show) noexcept {
  if (!menu::open) {
    return cheat::input.ShowCursor_hook.stdcall<std::int32_t>(show);
  }

  // Keep track of cursor show count while the menu is open.
  if (show == TRUE) {
    ++cheat::input.show_cursor_count;
  } else {
    --cheat::input.show_cursor_count;
  }

  return cheat::input.show_cursor_count;
}

HCURSOR WINAPI hooked_SetCursor(HCURSOR cursor) noexcept {
  if (!menu::open) {
    cheat::input.last_cursor = cursor;
    return cheat::input.SetCursor_hook.stdcall<HCURSOR>(cursor);
  }

  auto prev = cheat::input.last_cursor;

  cheat::input.last_cursor = cursor;

  return prev;
}

HCURSOR WINAPI hooked_GetCursor() noexcept {
  if (!menu::open) {
    return cheat::input.GetCursor_hook.stdcall<HCURSOR>();
  }

  return cheat::input.last_cursor;
}

BOOL WINAPI hooked_GetCursorInfo(PCURSORINFO pci) noexcept {
  if (!menu::open || pci == nullptr) {
    return cheat::input.GetCursorInfo_hook.stdcall<BOOL>(pci);
  }

  // Feed saved data into cursor info.
  pci->ptScreenPos = cheat::input.last_cursor_pos;

  if (pci->flags != CURSOR_SUPPRESSED && cheat::input.show_cursor_count >= 0) {
    pci->flags = CURSOR_SHOWING;
  }

  pci->hCursor = cheat::input.last_cursor;

  return cheat::input.GetCursorInfo_hook.stdcall<BOOL>(pci);
}

BOOL WINAPI hooked_ClipCursor(const RECT* rect) noexcept {
  cheat::input.last_cursor_clip = *rect;

  if (menu::open) {
    rect = nullptr;
  }

  return cheat::input.ClipCursor_hook.stdcall<BOOL>(rect);
}

BOOL WINAPI hooked_GetClipCursor(LPRECT rect) noexcept {
  if (!menu::open || rect == nullptr) {
    return cheat::input.GetClipCursor_hook.stdcall<BOOL>(rect);
  }

  *rect = cheat::input.last_cursor_clip;

  return TRUE;
}

LRESULT WINAPI wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
  auto stop_input = false;

  if (!cheat::attached) {
    return CallWindowProcW(cheat::input.orig_wndproc, hwnd, msg, wp, lp);
  }

  // NOTE: Calling `DefWindowProc` on these cause application hang (menu bar). IDK, Needs
  // testing.
  auto call_def = true;

  switch (msg) {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSCHAR: {
      call_def = false;
      break;
    }

    default: {
      break;
    }
  }

  switch (msg) {
    case WM_KEYDOWN:
    case WM_KEYUP: {
      auto is_key_down = msg == WM_KEYDOWN;

      uint8_t vk{};

      vk = wp;

      cheat::input.keys[vk] = is_key_down ? InputState::Down : InputState::Up;

      break;
    }

    default: {
      break;
    }
  }

  // Pass input to Dear ImGui.
  // NOTE: https://github.com/ocornut/imgui/issues/6895#issuecomment-1747239385
  {
    std::scoped_lock _{cheat::input.imgui_mutex};
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);
  }

  if (menu::open) {
    // Stop input if hovering menu.
    if (menu::allow_passthrough) {
      // TODO: Whitelist messages if needed (sizing, etc).

      auto& io = ImGui::GetIO();

      if (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput) {
        stop_input = true;
      }
    } else {
      stop_input = true;
    }
  }

  if (stop_input) {
    return call_def ? DefWindowProcW(hwnd, msg, wp, lp) : 0;
  }

  return CallWindowProcW(cheat::input.orig_wndproc, hwnd, msg, wp, lp);
}

void input_t::pre_check() noexcept {}

void input_t::post_check() noexcept { last_keys = keys; }

bool input_t::key_down(uint8_t vk) const noexcept { return keys[vk] == InputState::Down; }

bool input_t::key_pressed(uint8_t vk) const noexcept {
  return keys[vk] == InputState::Down && last_keys[vk] == InputState::Up;
}

void input_t::on_menu_open() noexcept {
  // Can we see the cursor?
  CURSORINFO ci{.cbSize = sizeof(CURSORINFO)};
  GetCursorInfo(&ci);

  // Hide the cursor.
  if (ci.flags == CURSOR_SHOWING) {
    if ((show_cursor_count = ShowCursor_hook.stdcall<std::int32_t>(FALSE) + 1) >= 0) {
      while (ShowCursor_hook.stdcall<std::int32_t>(FALSE) >= 0) {}
    }
  }

  // Save off cursor info.
  GetCursorPos_hook.stdcall<BOOL>(&last_cursor_pos);

  last_cursor = GetCursor_hook.stdcall<HCURSOR>();

  // Force a mouse event.
  PostMessageW(cheat::window, WM_SETCURSOR, (WPARAM)cheat::window,
               MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
}

void input_t::on_menu_close() noexcept {
  // Restore original show amount.
  auto show_count         = ShowCursor_hook.stdcall<std::int32_t>(TRUE);
  auto show_count_restore = show_count < show_cursor_count;

  if (show_count > show_cursor_count) {
    std::int32_t cur_show_count;

    while ((cur_show_count = ShowCursor_hook.stdcall<std::int32_t>(FALSE)) > show_cursor_count)
      show_count_restore = cur_show_count < show_cursor_count;
  }

  if (show_count_restore) {
    while (ShowCursor_hook.stdcall<std::int32_t>(TRUE) < show_cursor_count) {}
  }

  // Restore original cursor info.
  SetCursorPos_hook.stdcall<BOOL>(last_cursor_pos.x, last_cursor_pos.y);
  SetCursor_hook.stdcall<HCURSOR>(last_cursor);

  // Force a mouse event.
  PostMessageW(cheat::window, WM_SETCURSOR, (WPARAM)cheat::window,
               MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
}

void input_t::initialize() noexcept {
  SetCursorPos_hook = safetyhook::create_inline(reinterpret_cast<void*>(SetCursorPos),
                                                reinterpret_cast<void*>(hooked_SetCursorPos));

  GetCursorPos_hook = safetyhook::create_inline(reinterpret_cast<void*>(GetCursorPos),
                                                reinterpret_cast<void*>(hooked_GetCursorPos));

  ShowCursor_hook = safetyhook::create_inline(reinterpret_cast<void*>(ShowCursor),
                                              reinterpret_cast<void*>(hooked_ShowCursor));

  SetCursor_hook = safetyhook::create_inline(reinterpret_cast<void*>(SetCursor),
                                             reinterpret_cast<void*>(hooked_SetCursor));

  GetCursor_hook = safetyhook::create_inline(reinterpret_cast<void*>(GetCursor),
                                             reinterpret_cast<void*>(hooked_GetCursor));

  GetCursorInfo_hook = safetyhook::create_inline(reinterpret_cast<void*>(GetCursorInfo),
                                                 reinterpret_cast<void*>(hooked_GetCursorInfo));

  ClipCursor_hook = safetyhook::create_inline(reinterpret_cast<void*>(ClipCursor),
                                              reinterpret_cast<void*>(hooked_ClipCursor));

  GetClipCursor_hook = safetyhook::create_inline(reinterpret_cast<void*>(GetClipCursor),
                                                 reinterpret_cast<void*>(hooked_GetClipCursor));

  orig_wndproc = reinterpret_cast<WNDPROC>(
      SetWindowLongPtrW(cheat::window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndproc)));
}

void input_t::detach() noexcept {
  SetWindowLongPtrW(cheat::window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(orig_wndproc));

  GetClipCursor_hook = {};
  ClipCursor_hook    = {};
  GetCursorInfo_hook = {};
  GetCursor_hook     = {};
  SetCursor_hook     = {};
  ShowCursor_hook    = {};
  GetCursorPos_hook  = {};
  SetCursorPos_hook  = {};
}
