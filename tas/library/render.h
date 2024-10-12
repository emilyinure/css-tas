#pragma once

#include "safe_imgui_draw_cmds.h"

#include <imgui.h>

#include <cstdint>
#include <d3d9.h>
#include <dxgi.h>

struct render_vertex_t {
  float    x, y, z;
  float    rhw;
  uint32_t color;
};

class render_t {
public:
  glm::ivec2             screen_size{};
  IDXGISwapChain*        swap_chain{};
  ImFont*                tahoma_bold_13{};
  safe_imgui_draw_cmds_t tas_draw_cmds{};

  void initialize() noexcept;
  void detach() noexcept;
  void begin() noexcept;
  void finish() noexcept;
};
