#include "cheat.h"

#include "core/menu/menu.h"
#include "core/settings/settings.h"
#include "library/math.h"
#include "library/pe.h"
#include "library/utils.h"
#include "core/offsets/offsets.h"
#include "valve/client_frame_stage.h"
#include "valve/entities/base_entity.h"
#include "valve/entities/player.h"
#include "valve/usercmd.h"
#include "core/prediction/prediction.h"
#include "features/visual/esp.h"

#include <ShlObj.h>

void cheat::get_window_handle() {
  D3DDEVICE_CREATION_PARAMETERS creation_params;

  interfaces.d3d9_device->GetCreationParameters(&creation_params);

  window = creation_params.hFocusWindow;
}

void cheat::get_view_matrix() {
  view_setup_t setup;
  if (!interfaces.base_client->get_player_view(setup))
    return;

  view_matrix_t matrix1, matrix2, matrix3;
  interfaces.render_view->get_matrices_for_view(setup, &matrix1, &matrix2, &view_matrix,
                                                &matrix3);
}

bool cheat::initialize() {
  debug.open_console();

  debug.print("init misc");

  PWSTR localappdata_path{};
  if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &localappdata_path) !=
      S_OK) {
    return false;
  }

  dir = utils::str_utf16_to_8(localappdata_path);

  CoTaskMemFree(localappdata_path);

  dir /= "css_tas";

  // Create the cheat dirs if they don't exist.
  std::error_code ec{};

  if (std::filesystem::create_directories(dir, ec); ec) {
    return false;
  }

  if (std::filesystem::create_directories(dir / "configs", ec); ec) {
    return false;
  }

  debug.print("init interfaces");
  if (!interfaces.collect_interfaces())
    return false;

  debug.print("init window handle");
  get_window_handle();

  debug.print("init render");
  render.initialize();

  debug.print("init input");
  input.initialize();

  debug.print("init offsets");
  offsets.get_offsets();

  // debug.print("init player list");
  // player_list.initialize();

  // debug.print("init move sim");
  // movement_simulation.initialize();

  // debug.print("init proj aimbot");
  // projectile_aimbot.initialize();

  debug.print("init menu");
  menu::initialize();

  debug.print("init hooks");
  hooks.initialize();

  debug.print("flush modules");
  pe::flush_module_cache();

  attached = true;

  return true;
}

void cheat::detach() {
  attached = false;
  hooks.detach();
  debug.close_console();
  render.detach();
  input.detach();
}

void cheat::get_local_player() {
  local_player = interfaces.entity_list->get_entity<player_t>(
      interfaces.engine_client->get_local_player());
  if (local_player) {
    // eye_position = local_player->eye_position();
    local_weapon =
        interfaces.entity_list->get_entity<weapon_t>(local_player->active_weapon_handle());
  }
}

bool cheat::on_create_move(usercmd_t* current_cmd) {
  if (!current_cmd || !attached)
    return false;

  cmd = current_cmd;

  get_local_player();

  if (!local_player || local_player->life_state() != life_alive)
    return false;

  // movement.on_create_move();

  local_player->save_data("pre_pred", 10, 0);
  prediction.setup();

  tas.on_create_move();

  prediction.finish();

  local_player->restore_data("post_pred", 10, 0);

  // movement.correct_movement();

  // cmd->view_angles.y = math::normalize_angle(cmd->view_angles.y);
  // cmd->view_angles.x = std::clamp<float>(cmd->view_angles.x, -89.9f, 89.9f);
  // cmd->view_angles.z = std::clamp<float>(cmd->view_angles.z, -45.f, 45.f);

  return false;
}

void cheat::on_present() {
  // std::scoped_lock _{imgui_mutex};

  if (!attached)
    return;

  get_local_player();

  input.pre_check();

  render.begin();
  if (local_player) {
    // esp::run();
  }

  menu::on_present();
  input.post_check();

  render.finish();
}

void cheat::pre_frame_stage_notify(client_frame_stage_t stage) {}

void cheat::post_frame_stage_notify(client_frame_stage_t stage) {
  return;
  if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
    player_list.post_network_update();
  }
}
