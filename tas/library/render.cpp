#include "render.h"
#include "cheat.h"

#include <string>
#include <string_view>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include <misc/freetype/imgui_freetype.h>

DWORD g_prev_srgb{};

std::string get_system_font_path(std::string_view font_name) noexcept {
  char windows_directory[MAX_PATH]{};

  if (!GetWindowsDirectoryA(windows_directory, MAX_PATH)) {
    return {};
  }

  HKEY key{};

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts)",
                    0, KEY_READ, &key)) {
    return {};
  }

  DWORD index{};

  for (;;) {
    DWORD value_name_size = MAX_PATH;
    DWORD value_data_size = MAX_PATH;
    DWORD value_type{};
    char  value_name[MAX_PATH]{};
    char  value_data[MAX_PATH]{};

    auto error = RegEnumValueA(key, index++, value_name, &value_name_size, nullptr, &value_type,
                               reinterpret_cast<BYTE*>(value_data), &value_data_size);

    if (error == ERROR_NO_MORE_ITEMS) {
      break;
    }

    if (error != ERROR_SUCCESS || value_type != REG_SZ) {
      continue;
    }

    auto name =
        std::string_view{value_name, static_cast<std::string_view::size_type>(value_name_size)};

    if (name.compare(0, font_name.size(), font_name) != 0) {
      continue;
    }

    RegCloseKey(key);

    return std::string(windows_directory) + R"(\fonts\)" +
           std::string(value_data, value_data_size);
  }

  RegCloseKey(key);

  return {};
}

constexpr auto rgba_to_vec4(int32_t r, int32_t g, int32_t b, int32_t a = 255) noexcept {
  return ImColor(r, g, b, a).Value;
}

void render_t::initialize() noexcept {
  if (!ImGui::CreateContext()) {
    return;
  }

  auto& io       = ImGui::GetIO();
  io.IniFilename = nullptr; // TODO: Save this to hack dir too.
  io.ConfigFlags |=
      ImGuiConfigFlags_NoMouseCursorChange; // No mouse changes since we handle it ourselves.

  auto calibri = get_system_font_path("Calibri (TrueType)");
  if (calibri.empty()) {
    return;
  }

  auto tahoma = get_system_font_path("Tahoma Bold (TrueType)");
  if (tahoma.empty()) {
    return;
  }

  ImFontConfig font_config{};

  // Add menu font.
  io.Fonts->AddFontFromFileTTF(calibri.c_str(), 14);

  // Add overlay font.
  font_config.FontBuilderFlags |=
      ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;

  tahoma_bold_13 = io.Fonts->AddFontFromFileTTF(tahoma.c_str(), 13, &font_config);

  // Colors.
  {
    ImGui::StyleColorsDark();

    auto& style  = ImGui::GetStyle();
    auto& colors = style.Colors;

    colors[ImGuiCol_TitleBgActive]    = rgba_to_vec4(235, 94, 40, 200);
    colors[ImGuiCol_TitleBg]          = rgba_to_vec4(204, 197, 185);
    colors[ImGuiCol_Text]             = rgba_to_vec4(255, 252, 242);
    colors[ImGuiCol_WindowBg]         = rgba_to_vec4(37, 36, 34);
    colors[ImGuiCol_ChildBg]          = rgba_to_vec4(204, 197, 185);
    colors[ImGuiCol_FrameBg]          = rgba_to_vec4(64, 61, 57);
    colors[ImGuiCol_FrameBgHovered]   = rgba_to_vec4(235, 94, 40, 100);
    colors[ImGuiCol_FrameBgActive]    = rgba_to_vec4(235, 94, 40, 200);
    colors[ImGuiCol_SliderGrab]       = rgba_to_vec4(204, 197, 185);
    colors[ImGuiCol_SliderGrabActive] = rgba_to_vec4(235, 94, 40);
    // colors[ImGuiCol_FrameBgHovered] = rgba_to_vec4(45, 45, 45);
    // colors[ImGuiCol_FrameBgActive]  = rgba_to_vec4(35, 35, 35);
    colors[ImGuiCol_Button]           = rgba_to_vec4(64, 61, 57);
    colors[ImGuiCol_ButtonHovered]    = rgba_to_vec4(235, 94, 40, 100);
    colors[ImGuiCol_ButtonActive]     = rgba_to_vec4(235, 94, 40, 200);
    colors[ImGuiCol_CheckMark]        = rgba_to_vec4(235, 94, 40);
    colors[ImGuiCol_Header]           = rgba_to_vec4(80, 65, 90);
    colors[ImGuiCol_HeaderHovered]    = rgba_to_vec4(235, 94, 40, 100);
    colors[ImGuiCol_HeaderActive]     = rgba_to_vec4(235, 94, 40, 100);
    colors[ImGuiCol_Tab] =
        ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] =
        ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] =
        ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] =
        ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
  }
  // ImGui::StyleColorsDark();

  if (!ImGui_ImplWin32_Init(cheat::window))
    return;

  ImGui_ImplDX9_Init(cheat::interfaces.d3d9_device);
}

void render_t::detach() noexcept {
  ImGui_ImplDX9_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void render_t::begin() noexcept {
  cheat::interfaces.d3d9_device->GetRenderState(D3DRS_SRGBWRITEENABLE, &g_prev_srgb);

  cheat::interfaces.d3d9_device->SetRenderState(
      D3DRS_SRGBWRITEENABLE, FALSE); // this will disable linear to gamma correction

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();

  // Save off the screen size each frame. NOTE: This is grabbed from WinAPI's `GetClientRect`.
  auto& display_size = ImGui::GetIO().DisplaySize;
  screen_size        = {display_size.x, display_size.y};

  { // NOTE: https://github.com/ocornut/imgui/issues/6895#issuecomment-1747239385
    std::scoped_lock _{cheat::input.imgui_mutex};
    ImGui::NewFrame();
  }

  // Render thread-safe draw commands.
  tas_draw_cmds.draw(ImGui::GetBackgroundDrawList());
}

void render_t::finish() noexcept {
  ImGui::Render();
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

  cheat::interfaces.d3d9_device->SetRenderState(D3DRS_SRGBWRITEENABLE, g_prev_srgb);
}
