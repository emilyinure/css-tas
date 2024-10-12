#include "menu.h"
#include "cheat.h"
#include "core/settings/settings.h"
#include "features/tas.h"
#include "library/screen_position.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <winuser.h>

template <bool use_popup = true, bool show_tooltip = true>
bool slider_nudge(std::string_view id, auto& value, auto min, auto max, std::string_view fmt,
                  auto& step, auto step_min, auto step_max,
                  std::string_view step_fmt) noexcept {
  constexpr auto is_float = std::same_as<std::decay_t<decltype(step)>, float>;

  ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);

  if (use_popup) {
    if (ImGui::BeginPopup(id.data())) {
      if constexpr (is_float) {
        ImGui::SliderFloat("Step amount", &step, step_min, step_max, step_fmt.data(),
                           ImGuiSliderFlags_AlwaysClamp);
      } else {
        ImGui::SliderInt("Step amount", &step, step_min, step_max, step_fmt.data(),
                         ImGuiSliderFlags_AlwaysClamp);
      }

      ImGui::EndPopup();
    }
  }

  if constexpr (is_float) {
    ImGui::SliderFloat(id.data(), &value, min, max, fmt.data(), ImGuiSliderFlags_AlwaysClamp);
  } else {
    ImGui::SliderInt(id.data(), &value, min, max, fmt.data(), ImGuiSliderFlags_AlwaysClamp);
  }

  bool changed = ImGui::IsItemEdited();

  if (!ImGui::IsItemHovered() || static_cast<float>(step) <= 1e-6f) {
    return changed;
  }

  // Open the popup above.
  if (use_popup) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup(id.data());
    }

    if (show_tooltip) {
      if (ImGui::BeginTooltip()) {
        ImGui::TextUnformatted("Use '-', '+', Left arrow, Right arrow or your scroll wheel to "
                               "nudge the value.\nRight click for more options.");
        ImGui::EndTooltip();
      }
    }
  } else {
    if (show_tooltip) {
      if (ImGui::BeginTooltip()) {
        ImGui::TextUnformatted("Use '-', '+', Left arrow, Right arrow or your "
                               "scroll wheel to nudge the value.");
        ImGui::EndTooltip();
      }
    }
  }

  // Step the value.
  auto wheel = ImGui::GetIO().MouseWheel;
  auto inc = ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
             wheel > 0;
  auto dec = ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
             wheel < 0;

  if (inc || dec) {
    value += inc ? step : -step;
    value   = glm::clamp(value, min, max);
    changed = true;
  }

  return changed;
}

void menu::initialize() {}

void menu::on_present() {
  if (cheat::input.key_pressed(VK_HOME)) {
    open = !open;

    if (open) {
      cheat::input.on_menu_open();
    } else {
      cheat::input.on_menu_close();
    }
  }

  // Render software cursor.
  ImGui::GetIO().MouseDrawCursor = open;

  if (!open) {
    return;
  }

  ImGui::SetNextWindowSize({400.f, 500.f}, ImGuiCond_Once);
  if (!ImGui::Begin("TAS")) {
    ImGui::End();
    return;
  }

  static std::vector<segment_t> segments{};
  static properties_t           properties{};
  static int                    max_segments_rendered{};
  static bool                   set_playback_angles{};
  {
    std::shared_lock lock(cheat::tas.mutex);
    segments              = cheat::tas.get_segments();
    properties            = cheat::tas.get_properties();
    max_segments_rendered = cheat::tas.max_segments_rendered;
    set_playback_angles   = cheat::tas.set_playback_angles;
  }

  if (ImGui::BeginTabBar("##Tabs")) {
    if (ImGui::BeginTabItem("TAS")) {
      int& segment_idx = cheat::tas.selected_segment;

      if (ImGui::Button("Setup Prediction")) {
        segment_idx = 0;
        segments.clear();
        cheat::tas.needs_setup = true;
        segments.emplace_back();
      }

      bool disabled{};

      if (segments.empty()) {
        disabled = true;
      }

      if (!disabled) {
        ImGui::SameLine();
        if (ImGui::Button("Playback")) {
          cheat::tas.start_playback = true;
        }

        bool segment_changed{};

        static std::int32_t segment_nudge{1};
        slider_nudge<false>("Segment##Drag", segment_idx, 0,
                            static_cast<std::int32_t>(segments.size()) - 1, "%d", segment_nudge,
                            1, 1, "%d");

        if (ImGui::Button("Add##Segment")) {
          segments.emplace(segments.begin() + segment_idx + 1, segments.at(segment_idx));
          segments.at(segment_idx + 1).updated = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove selected##Segment")) {
          if (segments.size() > 1) {
            segments.erase(segments.begin() + segment_idx);
            if (segment_idx > 0)
              segment_idx--;
            segments.at(segment_idx).updated = true;
          }
        }

        segment_t& segment = segments[segment_idx];

        static std::int32_t frame_nudge{1};
        slider_nudge<false>("Frame##Drag", segment.selected_frame, 0,
                            static_cast<std::int32_t>(segment.get_frames()->size()) - 1, "%d",
                            frame_nudge, 1, 1, "%d");

        static int frame_set = 1;
        ImGui::PushItemWidth(50);
        ImGui::SliderInt("Count", &frame_set, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Add##Frame")) {
          frame_t frame;
          for (size_t i = 0; i < frame_set; i++)
            segment.frames.emplace(segment.frames.begin() + segment.selected_frame + 1, frame);
          segment_changed |= true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove##Frame")) {
          size_t frame_count    = segment.frames.size();
          int    frame_set_used = std::min<std::int32_t>(
              frame_set, static_cast<std::int32_t>(segment.frames.size() - 1));
          if (segment.frames.size() > 1) {
            int selection_end = segment.selected_frame + frame_set_used;
            selection_end     = std::min<std::int32_t>(
                selection_end, static_cast<std::int32_t>(segment.frames.size()));
            int selection_start = selection_end - frame_set_used;

            segment.frames.erase(segment.frames.begin() + selection_start,
                                 segment.frames.begin() + selection_end);

            segment.selected_frame = std::max<int>(0, selection_start - 1);
          }
          segment_changed |= true;
        }

        frame_t* frame              = segment.get_frame(segment.selected_frame);
        bool     properties_changed = false;
        if (ImGui::BeginTabBar("##Edit switcher")) {
          if (ImGui::BeginTabItem("Segment##Editor")) {
            ImGui::Combo("Movement Type", reinterpret_cast<std::int32_t*>(&segment.move_type),
                         "Strafe Towards(Slim)\0Pre Strafe\0Manual\0Strafe "
                         "side\0None\0");
            segment_changed |= ImGui::IsItemEdited();
            if (segment.move_type == strafe_side) {
              ImGui::Combo("Direction", reinterpret_cast<std::int32_t*>(&segment.direction),
                           "Left\0Right\0");
              segment_changed |= ImGui::IsItemEdited();
              ImGui::SliderFloat("Turn rate", &segment.strafe_side_data.turn_rate, 0, 10,
                                 "%.01f", ImGuiSliderFlags_AlwaysClamp);
              segment_changed |= ImGui::IsItemEdited();
            } else if (segment.move_type == strafe_towards) {
              segment_changed |= slider_nudge("Yaw", segment.yaw, -180.0f, 180.0f, "%.2f°",
                                              nudge_step, 0.0f, 1.0f, "%.2f");
              segment_changed |=
                  slider_nudge("Acceleration", segment.strafe_towards_data.accel_percentage,
                               -100.0f, 100.0f, "%.2f°", nudge_step, 0.0f, 1.0f, "%.2f");
            } else if (segment.move_type == pre_strafe) {
              ImGui::Combo("Direction", reinterpret_cast<std::int32_t*>(&segment.direction),
                           "Left\0Right\0");
              segment_changed |= ImGui::IsItemEdited();
            } else if (segment.move_type == manual) {
              segment_changed |= slider_nudge("Yaw", segment.yaw, -180.0f, 180.0f, "%.2f°",
                                              nudge_step, 0.0f, 1.0f, "%.2f");
              segment_changed |= slider_nudge("Pitch", segment.manual_data.pitch, -89.0f, 89.0f,
                                              "%.2f°", nudge_step, 0.0f, 1.0f, "%.2f");
              ImGui::Checkbox("Move forward", &segment.manual_data.move_forward);
              segment_changed |= ImGui::IsItemEdited();
              ImGui::Checkbox("Move left", &segment.manual_data.move_left);
              segment_changed |= ImGui::IsItemEdited();
              ImGui::Checkbox("Move right", &segment.manual_data.move_right);
              segment_changed |= ImGui::IsItemEdited();
              ImGui::Checkbox("Move backward", &segment.manual_data.move_backward);
              segment_changed |= ImGui::IsItemEdited();
            } else if (segment.move_type == strafe_accel) {
              ImGui::Combo("Direction", reinterpret_cast<std::int32_t*>(&segment.direction),
                           "Left\0Right\0");
              segment_changed |= ImGui::IsItemEdited();
              ImGui::SliderFloat("Starting turn rate",
                                 &segment.strafe_accel_data.starting_turn_rate, 0, 10, "%.01f",
                                 ImGuiSliderFlags_AlwaysClamp);

              segment_changed |= ImGui::IsItemEdited();
              segment_changed |=
                  slider_nudge("Acceleration", segment.strafe_accel_data.accel_percentage,
                               -100.0f, 100.0f, "%.2f°", nudge_step, 0.0f, 1.0f, "%.2f");
              segment_changed |= slider_nudge(
                  "Turn acceleration rate", segment.strafe_accel_data.turn_accel_rate, -100.0f,
                  100.0f, "%.2f°", nudge_step, 0.0f, 1.0f, "%.2f");
            }

            ImGui::Checkbox("Bunnyhop", &segment.bunnyhop);
            segment_changed |= ImGui::IsItemEdited();

            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("Frame##Editor")) {
            ImGui::Checkbox("Crouch", &frame->crouch);
            segment_changed |= ImGui::IsItemEdited();
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("Properties")) {
            ImGui::Checkbox("Ez Hop", &properties.ez_hop);
            properties_changed |= segment_changed || ImGui::IsItemEdited();
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("Rendering")) {
            ImGui::SliderInt("Max segments rendered", &max_segments_rendered, 0, 50);
            ImGui::EndTabItem();
          }
          if (ImGui::BeginTabItem("Misc")) {
            ImGui::Checkbox("Set playback angles", &set_playback_angles);
            ImGui::EndTabItem();
          }
          ImGui::EndTabBar();
        }

        if (properties_changed) {
          properties.updated = true;
        }

        if (segment_changed) {
          segment.updated = true;
        }
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Misc")) {
      ImGui::Checkbox("Input Passthrough", &allow_passthrough);

      // Configs.
      {
        static std::vector<std::string> cfg_stems{};

        auto cfg_path = cheat::dir / "configs";

        auto get_cfgs = [&cfg_path]() noexcept {
          cfg_stems.clear();

          std::error_code ec{};

          for (auto it = std::filesystem::directory_iterator(cfg_path, ec);
               !ec && it != std::filesystem::directory_iterator{}; it.increment(ec)) {
            if (!it->is_regular_file()) {
              continue;
            }

            if (it->path().filename().extension() != ".cfg") {
              continue;
            }

            cfg_stems.emplace_back(it->path().stem().string());
          }
        };

        // The first time the tab opens, refresh the items.
        if (static bool once{}; !std::exchange(once, true)) {
          get_cfgs();
        }

        static std::string selected_stem{};

        if (ImGui::BeginListBox("Configs")) {
          for (auto&& stem : cfg_stems) {
            if (ImGui::Selectable(stem.c_str(), stem == selected_stem)) {
              selected_stem = stem;
            }
          }

          ImGui::EndListBox();
        }

        auto listbox_size = ImGui::CalcItemWidth();

        std::optional<std::tuple<bool, std::string, std::string>> loaded_cfg{};

        if (ImGui::Button("Load")) {
          if (!selected_stem.empty()) {
            auto load_filepath = cfg_path / selected_stem;
            load_filepath.replace_extension(".cfg").make_preferred();

            if (auto file = std::ifstream(load_filepath)) {
              auto json = nlohmann::json::parse(file, nullptr, false);
              if (!json.is_discarded()) {
                // TODO: Type checks?
                if (json.contains("nudge_step")) {
                  nudge_step = json["nudge_step"];
                }

                if (json.contains("segments")) {
                  segments = json["segments"];
                }

                if (json.contains("properties")) {
                  properties = json["properties"];
                }

                if (json.contains("max_segments_rendered")) {
                  max_segments_rendered = json["max_segments_rendered"];
                }

                if (json.contains("set_playback_angles")) {
                  set_playback_angles = json["set_playback_angles"];
                }

                // TODO: Do we need a mutex for this?
                // auto& maps = cheat::tas.get_packed_maps();

                // if (json.contains("first_packed_map")) {
                //   auto bytes = json["first_packed_map"].get_binary();
                //   std::memcpy(&cheat::tas.get_packed_maps()[0]->map.pad[0],
                //   bytes.data(),
                //               bytes.size());
                // }

                loaded_cfg = {true, selected_stem, {}};
              } else {
                loaded_cfg = {false, selected_stem, "Failed to parse the config file"};
              }
            } else {
              loaded_cfg = {false, selected_stem, "Couldn't open the config file"};
            }
          } else {
            loaded_cfg = {false, {}, "No config selected"};
          }
        }

        ImGui::SameLine();

        if (ImGui::Button("Refresh")) {
          get_cfgs();
        }

        auto delete_txt_size = ImGui::CalcTextSize("Delete");

        ImGui::SameLine(listbox_size - delete_txt_size.x);

        std::optional<std::tuple<bool, std::string, std::string>> deleted_cfg{};

        if (ImGui::Button("Delete")) {
          if (!selected_stem.empty()) {
            auto delete_filepath = cfg_path / selected_stem;
            delete_filepath.replace_extension(".cfg").make_preferred();

            std::error_code ec{};

            if (std::filesystem::remove(delete_filepath, ec); !ec) {
              deleted_cfg = {true, selected_stem, {}};
            } else {
              deleted_cfg = {false, selected_stem, "Failed to delete config"};
            }
          } else {
            deleted_cfg = {false, {}, "No config selected"};
          }
        }

        static char save_cfg_buf[128]{};

        ImGui::InputText("CFG name", save_cfg_buf, sizeof(save_cfg_buf));

        std::optional<std::tuple<bool, std::string, std::string>> saved_cfg{};

        if (ImGui::Button("Save")) {
          std::string save_cfg_filename = save_cfg_buf;

          if (!save_cfg_filename.empty()) {
            // Replace characters in the string.
            std::ranges::replace_if(
                save_cfg_filename,
                [](char c) noexcept {
                  constexpr std::string_view bad_chars{R"(<>:"/\|?*)"};
                  return bad_chars.find(c) != std::string_view::npos;
                },
                '_');

            auto save_filepath = cfg_path / save_cfg_filename;
            save_filepath.replace_extension(".cfg").make_preferred();

            // Jail the path, then save it.
            if (save_filepath.parent_path() == cfg_path) {
              nlohmann::json cfg{};

              cfg["nudge_step"]            = nudge_step;
              cfg["segments"]              = segments;
              cfg["properties"]            = properties;
              cfg["max_segments_rendered"] = max_segments_rendered;
              cfg["set_playback_angles"]   = set_playback_angles;

              // TODO: Mutex?
              // auto& maps = cheat::tas.get_packed_maps();
              //
              // if (!maps.empty()) {
              //   auto first_map = maps.front()->map;
              //   auto first_map_bytes =
              //       std::vector(&first_map.pad[0],
              //       &first_map.pad[sizeof(first_map.pad)]);
              //   if (!first_map_bytes.empty()) {
              //     cfg["first_packed_map"] =
              //     nlohmann::json::binary_t{first_map_bytes};
              //   }
              // }

              if (auto file =
                      std::ofstream{save_filepath, std::ios::trunc | std::ios::binary}) {
                file << cfg;
                saved_cfg = {true, save_filepath.stem().string(), {}};
              } else {
                saved_cfg = {false, {}, "Couldn't open the config file"};
              }
            } else {
              saved_cfg = {false, {}, "Bad file path"};
            }
          } else {
            saved_cfg = {false, {}, "Empty filename"};
          }
        }

        static std::string notify_txt{}, last_notify_txt{};

        if (loaded_cfg) {
          auto& [loaded, name, reason] = *loaded_cfg;
          if (loaded) {
            notify_txt = std::format("Loaded config \"{}\".", name);

            // Force a full update.
            properties.updated = true;
          } else {
            if (name.empty()) {
              notify_txt = std::format("Failed to load config, {}!", reason);
            } else {
              notify_txt = std::format("Failed to load config \"{}\" ({})!", name, reason);
            }
          }

          get_cfgs();
        }

        if (deleted_cfg) {
          auto& [deleted, name, reason] = *deleted_cfg;
          if (deleted) {
            notify_txt = std::format("Deleted config \"{}\".", name);
          } else {
            if (name.empty()) {
              notify_txt = std::format("Failed to delete config, {}!", reason);
            } else {
              notify_txt = std::format("Failed to delete config \"{}\" ({})!", name, reason);
            }
          }

          get_cfgs();
        }

        if (saved_cfg) {
          auto& [saved, name, reason] = *saved_cfg;
          if (saved) {
            notify_txt = std::format("Saved config \"{}\".", name);
          } else {
            notify_txt = std::format("Failed to save config, {}!", reason);
          }

          save_cfg_buf[0] = '\0';
          get_cfgs();
        }

        // Show some text notifying the user for a few seconds about the config
        // action.
        static std::optional<std::chrono::high_resolution_clock::time_point> opt_wait_time{};

        // If the text changed, clear the time.
        if (notify_txt != last_notify_txt) {
          opt_wait_time = {};
        }

        if (!notify_txt.empty()) {
          auto now = std::chrono::high_resolution_clock::now();
          if (!opt_wait_time) {
            opt_wait_time = now + std::chrono::seconds(5);
          } else {
            ImGui::TextUnformatted(notify_txt.c_str());

            if (now > opt_wait_time) {
              opt_wait_time = {};
              notify_txt.clear();
            }
          }
        }

        last_notify_txt = notify_txt;
      }

      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Info")) {
      std::size_t num_frames{}, num_segments{};

      for (auto& segment : segments) {
        ++num_segments;
        num_frames += segment.get_frames()->size();
      }

      auto interval = cheat::interfaces.global_vars->tick_interval;

      ImGui::Text("Server tickrate: %u (%.6f)\n", static_cast<std::uint32_t>(1 / interval),
                  interval);
      ImGui::Text("Total segments: %u", num_segments);
      ImGui::Text("Total frames (ticks): %u", num_frames);
      ImGui::Text("Total frame time (seconds): %.2f",
                  interval * static_cast<float>(num_frames));

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  {
    std::unique_lock lock(cheat::tas.mutex);
    cheat::tas.get_segments()        = segments;
    cheat::tas.get_properties()      = properties;
    cheat::tas.max_segments_rendered = max_segments_rendered;
    cheat::tas.set_playback_angles   = set_playback_angles;
  }

  ImGui::End();
}
