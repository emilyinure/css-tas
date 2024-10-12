#include "esp.h"
#include "cheat.h"
#include "valve/entities/player.h"
#include "valve/entities/base_entity.h"
#include "library/math.h"
#include "valve/client_class_id.h"

#include <imgui.h>

bool get_player_box(player_t* player, area_t* box) {
  vector3_t bottom, top;

  const vector3_t origin = player->get_abs_origin();
  vector3_t       mins = player->mins(), maxs = player->maxs();

  mins = {origin.x, origin.y, origin.z + mins.z - 0.8f};
  maxs = {origin.x, origin.y, origin.z + maxs.z};

  if (!math::world_to_screen(mins, bottom) || !math::world_to_screen(maxs, top))
    return false;

  box->h = floorf(bottom.y - top.y);
  box->w = ceilf(box->h / 2.f);
  box->x = floorf(bottom.x - box->w / 2.f);
  box->y = ceilf(bottom.y - box->h);

  int screen_size_x = 0, screen_size_y = 0; // TODO: we should be caching these in renderer
  cheat::interfaces.engine_client->get_screen_size(screen_size_x, screen_size_y);

  if (box->h + box->y < 0)
    return false;

  if (box->x + box->w < 0)
    return false;

  if (box->y > screen_size_y)
    return false;

  if (box->x > screen_size_x)
    return false;

  return true;
}

void player_run(player_t* player) {
  auto networkable = player->get_networkable();
  if (networkable->is_dormant() || player->life_state() != life_alive)
    return;

  area_t box;
  if (!player->get_bounding_box(box, player->origin()))
    return;

  int alpha = player->team_number() == cheat::local_player->team_number() ? 100 : 255;

  ImColor color = ImColor(255, 255, 255, alpha);

  switch (player->team_number()) {
    case TEAM_BLUE:
      color = ImColor(131, 198, 242, alpha);
      break;
    case TEAM_RED:
      color = ImColor(227, 68, 71, alpha);
      break;
    default:
      break;
  }

  // cheat::render.draw_list->AddRect(ImVec2{box.x - 1, box.y - 1},
  //                                  ImVec2{box.x + (box.w + 1), box.y + (box.h + 1)},
  //                                  IM_COL32(0, 0, 0, 200));
  // cheat::render.draw_list->AddRect(ImVec2{box.x, box.y}, ImVec2{box.x + box.w, box.y +
  // box.h},
  //                                  color);
  // cheat::render.draw_list->AddRect(ImVec2{box.x + 1, box.y + 1},
  //                                  ImVec2{box.x + (box.w - 1), box.y + (box.h - 1)},
  //                                  IM_COL32(0, 0, 0, 200));

  player_info_t info;
  if (!cheat::interfaces.engine_client->get_player_info(
          player->get_networkable()->entity_index(), &info))
    return;

  // ImGui::PushFont(cheat::render.candara_14);
  auto text_size = ImGui::CalcTextSize(info.m_name);
  // cheat::render.draw_text_outlined(
  //     ImVec2{box.x + (box.w * 0.5f) - (text_size.x * 0.5f), box.y - (5.f + text_size.y)},
  //     IM_COL32(255, 255, 255, 255), IM_COL32(0, 0, 0, 255), info.m_name);
  // ImGui::PopFont();
}

void esp::run() {
  for (int i = 1; i < cheat::interfaces.entity_list->get_highest_entity_index(); ++i) {
    base_entity_t* ent = cheat::interfaces.entity_list->get_entity<base_entity_t>(i);

    if (!ent)
      continue;

    auto client_class = ent->get_networkable()->get_client_class();
    if (client_class->class_id == client_class_id::CCSPlayer) {
      player_run(reinterpret_cast<player_t*>(ent));
    }
  }
}
