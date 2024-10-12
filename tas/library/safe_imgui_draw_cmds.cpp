#include "safe_imgui_draw_cmds.h"

namespace ImGui {
  // This is unfortunate...
  void outline_text(ImDrawList* draw_list, ImFont* font, float size, const ImVec2& pos,
                    ImU32 fill_color, ImU32 outline_color, std::string_view str) noexcept {
    draw_list->AddText(font, size, {pos.x - 1, pos.y}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x - 1, pos.y - 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x, pos.y - 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x + 1, pos.y - 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x + 1, pos.y}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x + 1, pos.y + 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x, pos.y + 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x - 1, pos.y + 1}, outline_color, str.data());
    draw_list->AddText(font, size, {pos.x, pos.y}, fill_color, str.data());
  }

  void outline_text(ImDrawList* draw_list, const ImVec2& pos, ImU32 fill, ImU32 outline,
                    std::string_view str) noexcept {
    outline_text(draw_list, nullptr, 0.f, pos, fill, outline, str);
  }

  void outline_text(ImDrawList* draw_list, ImFont* font, const ImVec2& pos, ImU32 fill,
                    ImU32 outline, std::string_view str) noexcept {
    outline_text(draw_list, font, 0.f, pos, fill, outline, str);
  }

  vector2_t outlined_text_size(const char* text, const char* text_end,
                               bool hide_text_after_double_hash, float wrap_width) noexcept {
    auto size = CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);

    auto res = vector2_t(size.x, size.y);
    res += 2;

    return res;
  }
} // namespace ImGui

void safe_imgui_draw_cmds_t::line(const vector2_t& start, const vector2_t& end, ImU32 color,
                                  float thickness, bool antialiased) noexcept {
  cmds.emplace_back(line_t{{start.x, start.y}, {end.x, end.y}, color, thickness, antialiased});
}

void safe_imgui_draw_cmds_t::rect_filled(const glm::vec2& pos, const vector2_t& size,
                                         ImU32 color, float rounding) noexcept {
  auto max = pos + size;
  cmds.emplace_back(rect_filled_t{{pos.x, pos.y}, {max.x, max.y}, color, rounding});
}

void safe_imgui_draw_cmds_t::rect(const vector2_t& pos, const vector2_t& size, ImU32 color,
                                  float rounding, float thickness) noexcept {
  auto max = pos + size;
  cmds.emplace_back(rect_t{{pos.x, pos.y}, {max.x, max.y}, color, rounding, thickness});
}

void safe_imgui_draw_cmds_t::text(ImFont* font, const vector2_t& pos, ImU32 color,
                                  const std::string& str, bool center) noexcept {
  cmds.emplace_back(text_t{font, {pos.x, pos.y}, color, 0, false, center, str});
}

void safe_imgui_draw_cmds_t::text(const vector2_t& pos, ImU32 color, const std::string& str,
                                  bool center) noexcept {
  text(nullptr, pos, color, str, center);
}

void safe_imgui_draw_cmds_t::text(ImFont* font, const vector2_t& pos, ImU32 fill_color,
                                  ImU32 outline_color, const std::string& str,
                                  bool center) noexcept {
  cmds.emplace_back(text_t{font, {pos.x, pos.y}, fill_color, outline_color, true, center, str});
}

void safe_imgui_draw_cmds_t::text(const vector2_t& pos, ImU32 fill_color, ImU32 outline_color,
                                  const std::string& str, bool center) noexcept {
  text(nullptr, pos, fill_color, outline_color, str, center);
}

void safe_imgui_draw_cmds_t::flush() noexcept {
  {
    std::scoped_lock _{mtx};
    std::swap(safe_cmds, cmds);
  }

  cmds.clear();
}

void safe_imgui_draw_cmds_t::clear() noexcept {
  {
    std::scoped_lock _{mtx};
    safe_cmds.clear();
  }

  cmds.clear();
}

void safe_imgui_draw_cmds_t::draw(ImDrawList* draw_list) noexcept {
  std::scoped_lock _{mtx};

  for (auto&& cmd : safe_cmds) {
    if (auto* line = std::get_if<line_t>(&cmd)) {
      if ((line->color & IM_COL32_A_MASK) == 0) {
        break;
      }

      auto old_flags = draw_list->Flags;

      if (line->antialiased) {
        draw_list->Flags |=
            ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex;
      } else {
        draw_list->Flags &=
            ~(ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex);
      }

      draw_list->PathLineTo(line->start);
      draw_list->PathLineTo(line->end);
      draw_list->PathStroke(line->color, ImDrawFlags_None, line->thickness);

      draw_list->Flags = old_flags;
    } else if (auto* rect_filled = std::get_if<rect_filled_t>(&cmd)) {
      // TODO: Support more args (anti alias, rounding sides, etc).
      draw_list->AddRectFilled(rect_filled->min, rect_filled->max, rect_filled->color,
                               rect_filled->rounding, ImDrawFlags_None);
    } else if (auto* rect = std::get_if<rect_t>(&cmd)) {
      // TODO: Support more args (anti alias, rounding sides, etc).
      draw_list->AddRect(rect->min, rect->max, rect->color, rect->rounding, ImDrawFlags_None,
                         rect->thickness);
    } else if (auto* text = std::get_if<text_t>(&cmd)) {
      if (text->center) {
        text->pos.x -= glm::round(ImGui::outlined_text_size(text->str.c_str()).x / 2.f);
      }

      if (text->outlined) {
        ImGui::outline_text(draw_list, text->font, 0.0f, text->pos, text->fill_color,
                            text->outline_color, text->str);
      } else {
        draw_list->AddText(text->font, 0.0f, text->pos, text->fill_color, text->str.c_str());
      }
    }
  }
}