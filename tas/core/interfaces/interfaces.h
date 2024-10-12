#pragma once

#include "valve/entity_list.h"
#include "valve/base_client.h"
#include "valve/engine_client.h"
#include "valve/global_vars.h"
#include "valve/prediction.h"
#include "valve/console.h"
#include "valve/render_view.h"
#include "valve/tracing/engine_trace.h"
#include "valve/view_setup.h"
#include "valve/model_info.h"
#include "valve/input.h"
#include "valve/bsp.h"

#include <d3d9.h>

class interfaces_t {
public:
  base_client_t*        base_client   = nullptr;
  engine_client_t*      engine_client = nullptr;
  void*                 client_mode   = nullptr;
  cinput_t*             input{};
  ccsgamerules_t**      game_rules{};
  entity_list_t*        entity_list   = nullptr;
  prediction_manager_t* prediction    = nullptr;
  game_movement_t*      game_movement = nullptr;
  move_helper_t*        move_helper   = nullptr;
  global_vars_t*        global_vars   = nullptr;
  cvar_t*               console       = nullptr;
  IDirect3DDevice9*     d3d9_device   = nullptr;
  engine_trace_t*       engine_trace  = nullptr;
  render_view_t*        render_view   = nullptr;
  void*                 engine_vgui   = nullptr;
  model_info_t*         model_info    = nullptr;
  collision_bsp_data_t* bsp_data{};

  bool collect_interfaces();
};
