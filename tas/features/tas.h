#pragma once

#include "valve/prediction.h"
#include "library/math.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <vector>
#include <shared_mutex>

#include "tas/input.h"
#include "tas/segment.h"

struct packed_map_t {
  uint8_t pad[1000];

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(packed_map_t, pad);
};

struct packed_map_holder {
  packed_map_holder() {}
  packed_map_t map{};
};

struct properties_t {
  bool ez_hop  = true;
  bool updated = false;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(properties_t, ez_hop, updated)
};

struct draw_data_t {
  int flags = 0;
};

class tas_t {
  float cur_time{};
  bool  setup{};
  bool  needs_update{};

  int update_from{};

  std::vector<input_data_t>                       input{};
  std::vector<move_data_t>                        move_data{};
  std::vector<draw_data_t>                        draw_data{};
  std::vector<segment_t>                          segments{};
  std::vector<segment_t>                          used_segments{segments};
  std::vector<std::unique_ptr<packed_map_holder>> packed_maps{};

  properties_t properties{};

  bool   in_playback{};
  size_t playback_tick{};

  void setup_prediction(usercmd_t* cmd);
  void process_movement(move_data_t* move_data);
  void setup_move_data_input(move_data_t* move_data, input_data_t* input);
  void setup_command_input(usercmd_t* command, move_data_t* move_data);
  void handle_prediction();
  void generate_input_for_segment(segment_t* segment, int& tick);
  void generate_input();

public:
  int               selected_segment{};
  bool              start_playback{};
  bool              needs_setup{};
  int               max_segments_rendered{5};
  std::shared_mutex mutex{};
  bool              set_playback_angles{};
  auto&             get_segments() { return segments; }
  auto&             get_properties() { return properties; }
  auto&             get_packed_maps() noexcept { return packed_maps; }
  void              draw_frame(move_data_t*, draw_data_t*) noexcept;
  void              draw_segment(segment_t*, bool selected) noexcept;
  void              draw() noexcept;
  void              playback();
  void              on_create_move();
};
