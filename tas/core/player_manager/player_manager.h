#pragma once

#include "library/math.h"

#include <array>
#include <deque>

class player_t;

class player_tick_manager_t;
class player_manager_t;

class player_tick_record_t {
public:
  float simulation_time = 0;
  int   flags           = 0;
  int   lag             = 0;

  vector3_t origin;
  vector3_t abs_origin;
  vector3_t velocity;
  vector3_t eye_angles;
  vector3_t unstuck_origin;
  vector3_t ground_input_wish_dir;
  vector3_t air_input_wish_dir;

  std::array<matrix_3x4_t, 128> bones;
  bool                          bones_setup = false;

  player_tick_manager_t* manager;
  player_tick_record_t(player_tick_manager_t*);
};

class player_tick_manager_t {
public:
  player_tick_manager_t() {}

  void reset() {
    records.clear();
    manager = nullptr;
  }

  player_manager_t*                manager = nullptr;
  void                             update(player_manager_t*);
  std::deque<player_tick_record_t> records = {};
};

class player_manager_t {
  void check_targeting();
  void get_unstuck_position();

public:
  bool is_target = false;

  uint32_t              client_class_id = 0; // TODO: implement this caching
  player_t*             player;
  player_tick_manager_t tick_manager{};
  float                 simulation_time = 0;
  float                 spawn_time      = 0;
  int                   lag             = 0;

  bool      prediction_successful  = false;
  vector3_t predicted_origin       = vector3_t(0, 0, 0);
  vector3_t predicted_velocity     = vector3_t(0, 0, 0);
  bool      needs_input_prediction = true;

  vector3_t origin;
  vector3_t abs_origin;
  vector3_t velocity;
  vector3_t eye_angles;
  vector3_t unstuck_origin;
  vector3_t ground_input_wish_dir;
  vector3_t air_input_wish_dir;

  void reset() {
    is_target       = false;
    client_class_id = 0;
    player          = nullptr;
    tick_manager.reset();
    simulation_time = 0.f;
    spawn_time      = 0.f;
  }

  void soft_reset() {}

  bool check_stuck(const vector3_t& origin);

  void post_network_update();

  void update(player_t* player_) {
    if (player != player_) {
      reset();
      player = player_;
    }
  }
};

class player_list_t {
public:
  std::array<player_manager_t, 128> player_lists = {};
  bool                              initialize();
  void                              post_network_update();
};
