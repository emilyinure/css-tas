#pragma once

#include "library/math.h"

#include "nlohmann/json.hpp"

#include "input.h"


struct frame_t {
  bool crouch = false;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(frame_t, crouch)
};

enum movement_type_t : int {
  strafe_towards,
  pre_strafe,
  manual,
  strafe_side,
  strafe_accel,
  none
};


struct strafe_accel_data_t {
  float starting_turn_rate{5.f};
  float accel_percentage{100.f};
  float turn_accel_rate{};
};

struct strafe_side_data_t {
  float turn_rate{0.f};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(strafe_side_data_t, turn_rate)
};

struct strafe_towards_data_t {
  float accel_percentage{100.f};
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(strafe_towards_data_t, accel_percentage)
};

struct manual_data_t {
  bool  move_left{false};
  bool  move_right{false};
  bool  move_forward{false};
  bool  move_backward{false};
  float pitch{};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(manual_data_t, move_left, move_right, move_forward,
                                 move_backward, pitch)
};

struct segment_t {
  segment_t() = default;

  frame_t*              get_frame(size_t frame) { return &frames[frame]; }
  std::vector<frame_t>* get_frames() { return &frames; }

  manual_data_t         manual_data{};
  strafe_towards_data_t strafe_towards_data{};
  strafe_side_data_t    strafe_side_data{};
  strafe_accel_data_t   strafe_accel_data{};

  bool                 updated{true};
  int                  start_tick{};
  int                  end_tick{};
  std::vector<frame_t> frames{{}};
  movement_type_t      move_type{none};
  int                  selected_frame{};
  uint32_t             abs_selected_frame{};

  bool                 bunnyhop{};
  bool                 crouch_hop{};
  movement_direction_t direction{left}; // this is reused for several different modes
  float                yaw{};           // this is reused for several different modes

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(segment_t, manual_data, strafe_side_data, frames, move_type,
                                 bunnyhop, crouch_hop, direction, yaw)
};

