#pragma once

#include "math.h"

#include <imgui.h>

#include <string>
#include <string_view>
#include <mutex>
#include <variant>
#include <vector>

namespace ImGui {
  extern void outline_text(ImDrawList* draw_list, ImFont* font, float size, const ImVec2& pos,
                           ImU32 fill_color, ImU32 outline_color,
                           std::string_view str) noexcept;

  extern void outline_text(ImDrawList* draw_list, const ImVec2& pos, ImU32 fill_color,
                           ImU32 outline_color, std::string_view str) noexcept;

  extern void outline_text(ImDrawList* draw_list, ImFont* font, const ImVec2& pos,
                           ImU32 fill_color, ImU32 outline_color,
                           std::string_view str) noexcept;

  vector2_t outlined_text_size(const char* text, const char* text_end = nullptr,
                               bool  hide_text_after_double_hash = false,
                               float wrap_width                  = -1.0f) noexcept;
} // namespace ImGui

class safe_imgui_draw_cmds_t {
public:
  safe_imgui_draw_cmds_t() noexcept = default;

  void line(const vector2_t& start, const vector2_t& end, ImU32 color, float thickness = 1.0f,
            bool antialiased = true) noexcept;

  void rect_filled(const vector2_t& pos, const vector2_t& size, ImU32 color,
                   float rounding = 0.0f) noexcept;

  void rect(const vector2_t& pos, const vector2_t& size, ImU32 color, float rounding = 0.0f,
            float thickness = 1.0f) noexcept;

  void text(ImFont* font, const vector2_t& pos, ImU32 color, const std::string& str,
            bool center = false) noexcept;

  void text(const vector2_t& pos, ImU32 color, const std::string& str,
            bool center = false) noexcept;

  void text(ImFont* font, const vector2_t& pos, ImU32 fill_color, ImU32 outline_color,
            const std::string& str, bool center = false) noexcept;

  void text(const vector2_t& pos, ImU32 fill_color, ImU32 outline_color, const std::string& str,
            bool center = false) noexcept;

  // Clear all commands.
  void clear() noexcept;

  // Flush the temporary commands.
  void flush() noexcept;

  // Send commands to draw list.
  void draw(ImDrawList* draw_list) noexcept;

private:
  struct line_t {
    ImVec2 start{};
    ImVec2 end{};
    ImU32  color{};
    float  thickness{};
    bool   antialiased{};
  };

  struct rect_filled_t {
    ImVec2 min{};
    ImVec2 max{};
    ImU32  color{};
    float  rounding{};
  };

  struct rect_t {
    ImVec2 min{};
    ImVec2 max{};
    ImU32  color{};
    float  rounding{};
    float  thickness{};
  };

  struct text_t {
    ImFont*     font{};
    ImVec2      pos{};
    ImU32       fill_color{};
    ImU32       outline_color{};
    bool        outlined{};
    bool        center{};
    std::string str{};
  };

  std::mutex                                                       mtx{};
  std::vector<std::variant<line_t, rect_filled_t, rect_t, text_t>> cmds{}, safe_cmds{};
};