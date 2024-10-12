#pragma once

#include "core/input/input.h"
#include "core/prediction/prediction.h"
#include "core/interfaces/interfaces.h"
#include "core/player_manager/player_manager.h"
#include "core/debug/debug.h"
#include "features/movement/movement.h"
#include "features/tas.h"
#include "features/compression_handler.h"
#include "hooks/hooks.h"
#include "library/math.h"
#include "valve/client_frame_stage.h"
#include "library/render.h"

#include <filesystem>

class usercmd_t;
class player_t;
class weapon_t;

namespace cheat {
  inline debug_t      debug;
  inline interfaces_t interfaces;
  inline hooks_t      hooks;

  inline usercmd_t* cmd;
  inline player_t*  local_player;
  inline weapon_t*  local_weapon;

  inline render_t render;
  inline input_t  input;

  inline HWND window;

  inline prediction_t              prediction;
  inline movement_t                movement;
  inline player_list_t             player_list;
  inline tas_t                     tas;
  inline prediction_netvar_manager compression_manager;

  inline view_matrix_t view_matrix;

  inline std::shared_mutex render_mutex = {};

  inline bool attached = false;

  inline vector3_t eye_position = vector3_t(0, 0, 0);

  inline std::filesystem::path dir{}; // Directory for saving stuff to.

  bool initialize();
  void get_window_handle();
  void get_view_matrix();
  void detach();
  void get_local_player();
  bool on_create_move(usercmd_t* cmd);
  void on_present();

  void pre_frame_stage_notify(client_frame_stage_t stage);
  void post_frame_stage_notify(client_frame_stage_t stage);
} // namespace cheat
