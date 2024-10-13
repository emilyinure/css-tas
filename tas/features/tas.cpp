#include "tas.h"

#include "cheat.h"
#include "library/math.h"
#include "movement/movement.h"
#include "valve/bsp.h"
#include "valve/entities/player.h"
#include "valve/prediction.h"
#include "valve/prediction_copy.h"
#include "valve/usercmd.h"

#include <imgui.h>

#include <format>
#include <shared_mutex>

void tas_t::setup_prediction(usercmd_t* cmd) {
  if (!needs_setup)
    return;

  // This sucks.
  // void* buf = malloc(1300 + sizeof(base_trigger_t));
  // auto* foo = static_cast<base_trigger_t*>(buf);

  // auto* test = new(std::nothrow) trigger_teleport_t;

  cheat::local_player->current_cmd() = cmd;
  cheat::interfaces.move_helper->set_host(cheat::local_player);
  setup       = true;
  needs_setup = false;
  move_data.clear();
  packed_maps.clear();
  move_data_t move_data{};
  cheat::interfaces.prediction->setup_move(cheat::local_player, cmd,
                                           cheat::interfaces.move_helper, &move_data);

  // ray_t ray;
  // ray.initialize(move_data.abs_origin, move_data.abs_origin - vector3_t(0, 0,
  // 2),
  //                cheat::local_player->mins(), cheat::local_player->maxs());
  // trace_filter_t filter(cheat::local_player);
  // trace_t        trace;

  // cheat::interfaces.engine_trace->trace_ray(ray, MASK_SOLID, &filter,
  // &trace); move_data.abs_origin = trace.end;

  // move_data.velocity = {};
  this->move_data.push_back(move_data);
  auto& packed_map = this->packed_maps.emplace_back(std::make_unique<packed_map_holder>());
  prediction_copy_t copy(PC_EVERYTHING, &packed_map->map, PC_DATA_PACKED, cheat::local_player,
                         PC_DATA_NORMAL);
  copy.transfer_data("", cheat::local_player->entity_index(),
                     cheat::local_player->get_data_map());
}

usercmd_t dummy;

void tas_t::process_movement(move_data_t* move_data) {
  cheat::interfaces.move_helper->set_host(cheat::local_player);
  cheat::interfaces.prediction->is_in_prediction     = true;
  cheat::interfaces.prediction->first_time_predicted = false;

  cheat::interfaces.global_vars->cur_time   = cur_time;
  cheat::interfaces.global_vars->frame_time = cheat::interfaces.global_vars->tick_interval;

  if (properties.ez_hop)
    cheat::local_player->stamina() = 0;

  { // pre think
    if (!(cheat::local_player->flags() & FL_ONGROUND))
      cheat::local_player->fall_velocity() = -move_data->velocity.z;
  }

  cheat::interfaces.game_movement->proccess_movement(cheat::local_player, move_data);

  { // post think
    if (!!(cheat::local_player->flags() & FL_ONGROUND))
      cheat::local_player->fall_velocity() = 0.f;

    // pray to the trap god that this is filled in
    auto* game_rules   = *cheat::interfaces.game_rules;
    auto* view_vectors = game_rules->get_view_vectors();

    if (!!(cheat::local_player->flags() & FL_DUCKING)) {
      cheat::local_player->set_collision_bounds(view_vectors->duck_hull_min,
                                                view_vectors->duck_hull_max);
    } else {
      cheat::local_player->set_collision_bounds(view_vectors->hull_min, view_vectors->hull_max);
    }
  }
  // cheat::interfaces.prediction->finish_move(cheat::local_player, &dummy,
  // move_data);
}

float accel_to_yaw(float target_yaw, vector3_t velocity, float& forward_move, float& side_move,
                   float percent, bool on_ground) {

  static convar_t* sv_accelerate = cheat::interfaces.console->find_var("sv_accelerate");
  static convar_t* sv_stopspeed  = cheat::interfaces.console->find_var("sv_stopspeed");
  static convar_t* sv_friction   = cheat::interfaces.console->find_var("sv_friction");

  velocity.z       = 0;
  auto speed       = glm::length(velocity);
  auto view_rad    = math::normalize_rad(math::degrees_to_radian(target_yaw));
  auto vel_yaw_rad = glm::atan(velocity.y, velocity.x);

  float wishspeed = glm::min(cheat::local_player->max_speed(),
                             400.0f); // this can change, if a style has higher
                                      // max speed we'll implement
  if (on_ground) {
    wishspeed = std::sqrt(std::pow(450.f, 2) + std::pow(450.f, 2));
    wishspeed = std::min(cheat::local_player->max_speed(), wishspeed);

    float surface_friction = cheat::local_player->surface_friction();

    float stop_speed = sv_stopspeed->value.float_value;
    float control    = (speed < stop_speed) ? stop_speed : speed;

    float drop = control * sv_friction->value.float_value *
                 cheat::interfaces.global_vars->tick_interval * surface_friction;

    speed = ((speed - drop) < 0.f) ? 0.f : speed - drop;
  }

  double theta = 0.f;

  double const_speed_theta = movement_t::const_speed_theta(speed, wishspeed, on_ground);
  if (percent > 0.f) {
    double accel_theta = movement_t::max_accel_theta(speed, wishspeed, on_ground);
    theta = math::normalize_rad(((accel_theta - const_speed_theta) * glm::abs(percent)) +
                                const_speed_theta);
  } else {
    double deccel_theta = movement_t::max_deccel_theta(speed, wishspeed, on_ground);
    theta = math::normalize_rad(((deccel_theta - const_speed_theta) * glm::abs(percent)) +
                                const_speed_theta);
  }

  if (theta == 0.0 || theta == std::numbers::pi_v<double>) {
    theta = math::normalize_rad(view_rad - vel_yaw_rad + theta);
  } else
    theta = copysign(theta, math::normalize_rad(view_rad - vel_yaw_rad));

  if (speed < 0.1f) {
    side_move    = 0.f;
    forward_move = 400.f;
    return target_yaw;
  }

  side_move = static_cast<float>(theta < 0) * 400.0f + static_cast<float>(theta > 0) * -400.0f;
  forward_move = on_ground ? 400.f : 0.f;

  double buttons_phi = -glm::atan(side_move, forward_move);
  return math::normalize_angle(
      math::radian_to_degrees(math::normalize_rad(vel_yaw_rad - buttons_phi + theta)));
}

float max_ground_accel_to_direction(float yaw, vector3_t velocity, float& forward_move,
                                    float& side_move, bool left) {
  static convar_t* sv_accelerate = cheat::interfaces.console->find_var("sv_accelerate");
  static convar_t* sv_stopspeed  = cheat::interfaces.console->find_var("sv_stopspeed");
  static convar_t* sv_friction   = cheat::interfaces.console->find_var("sv_friction");

  velocity.z   = 0;
  float speed  = glm::length(velocity);
  forward_move = 400.f;
  side_move    = 0.f;

  float wishspeed = std::sqrt(std::pow(forward_move, 2) + std::pow(side_move, 2));
  wishspeed       = std::min(cheat::local_player->max_speed(), wishspeed);

  float surface_friction = cheat::local_player->surface_friction();

  float stop_speed = sv_stopspeed->value.float_value;
  float control    = (speed < stop_speed) ? stop_speed : speed;

  float drop = control * sv_friction->value.float_value *
               cheat::interfaces.global_vars->tick_interval * surface_friction;

  float newspeed = ((speed - drop) < 0.f) ? 0.f : speed - drop;

  double buttons_phi = -atan2(side_move, forward_move);

  double vel_yaw_rad = atan2(velocity.y, velocity.x);

  double theta = movement_t::max_accel_theta(newspeed, wishspeed, true);

  if (theta <= 0.0)
    return yaw;

  if (!left)
    theta *= -1.f;

  return math::normalize_angle(
      math::radian_to_degrees(math::normalize_rad(vel_yaw_rad - buttons_phi + theta)));
}

// float setup_move_data_strafe_accel(move_data_t* move_data, input_data_t* input) {
//
//   vector3_t        velocity      = move_data->velocity;
//   static convar_t* sv_accelerate = cheat::interfaces.console->find_var("sv_accelerate");
//   static convar_t* sv_stopspeed  = cheat::interfaces.console->find_var("sv_stopspeed");
//   static convar_t* sv_friction   = cheat::interfaces.console->find_var("sv_friction");
//
//   velocity.z       = 0;
//   auto speed       = glm::length(velocity);
//   auto vel_yaw_rad = glm::atan(velocity.y, velocity.x);
//
//   float wishspeed = glm::min(cheat::local_player->max_speed(),
//                              400.0f); // this can change, if a style has higher
//                                       // max speed we'll implement
//
//   double theta = 0.f;
//
//   double const_speed_theta = movement_t::const_speed_theta(speed, wishspeed, on_ground);
//   if (percent > 0.f) {
//     double accel_theta = movement_t::max_accel_theta(speed, wishspeed, on_ground);
//     theta = math::normalize_rad(((accel_theta - const_speed_theta) * glm::abs(percent)) +
//                                 const_speed_theta);
//   } else {
//     double deccel_theta = movement_t::max_deccel_theta(speed, wishspeed, on_ground);
//     theta = math::normalize_rad(((deccel_theta - const_speed_theta) * glm::abs(percent)) +
//                                 const_speed_theta);
//   }
//
//   if (speed < 0.1f) {
//     move_data->side_move    = 0.f;
//     move_data->forward_move = 400.f;
//     return;
//   }
//
//   double buttons_phi = -glm::atan(move_data->side_move, move_data->forward_move);
//   return math::normalize_angle(
//       math::radian_to_degrees(math::normalize_rad(vel_yaw_rad - buttons_phi + theta)));
// }

void tas_t::setup_move_data_input(move_data_t* move_data, input_data_t* input) {
  move_data->forward_move = input->forward_move;
  move_data->side_move    = input->side_move;
  move_data->up_move      = input->up_move;

  move_data->view_angles.y += input->yaw;
  move_data->view_angles.x = input->pitch;

  move_data->buttons = 0;
  move_data->buttons = input->buttons;

  if (!!(cheat::local_player->flags() & FL_ONGROUND) && input->bhop) {
    move_data->buttons |= IN_JUMP;
  }

  switch (input->yaw_change_type) {
    case yaw_type::toward:
      move_data->view_angles.y = accel_to_yaw(
          input->yaw, move_data->velocity, move_data->forward_move, move_data->side_move,
          input->accel_percentage,
          !(!(cheat::local_player->flags() & FL_ONGROUND) || (input->buttons & IN_JUMP)));
      break;
    case yaw_type::set:
      move_data->view_angles.y = input->yaw;
      break;
    case yaw_type::relative:
      move_data->view_angles.y += input->yaw;
      break;
    case yaw_type::pre_strafe:
      move_data->view_angles.y = max_ground_accel_to_direction(
          move_data->view_angles.y, move_data->velocity, move_data->forward_move,
          move_data->side_move, input->direction == left);
      break;
    case yaw_type::relative_accel:

      break;
  }
  if (move_data->side_move < -1)
    move_data->buttons |= IN_MOVELEFT;
  if (move_data->side_move > 1)
    move_data->buttons |= IN_MOVERIGHT;
  if (move_data->forward_move > 1)
    move_data->buttons |= IN_FORWARD;
  if (move_data->forward_move < -1)
    move_data->buttons |= IN_BACK;
}

void tas_t::setup_command_input(usercmd_t* command, move_data_t* move_data) {
  command->forward_move = move_data->forward_move;
  command->side_move    = move_data->side_move;
  command->up_move      = move_data->up_move;

  command->view_angles.y = move_data->view_angles.y;
  command->view_angles.x = move_data->view_angles.x;

  command->buttons = move_data->buttons;
}

void tas_t::handle_prediction() {
  if (input.empty() || !setup || move_data.empty() || !needs_update)
    return;

  cheat::local_player->current_cmd() = &dummy;

  // if (update_from > 0 && update_from < packed_maps.size()) {
  //   packed_maps.erase(packed_maps.begin() + update_from, packed_maps.end());
  // }
  // size_t last_i = 0;
  // for (size_t i = 0; i < packed_maps.size(); i++) {
  //   if (packed_maps[i])
  //     last_i = i;
  // }

  // if (last_i < update_from) {
  //   update_from = last_i;
  // }
  update_from = 0; // TODO: find what var is missing from the datamap and we can undo this

  if (update_from < draw_data.size()) {
    draw_data.erase(draw_data.begin() + update_from, draw_data.end());
  }

  if (update_from < move_data.size() - 1) {
    move_data.erase(move_data.begin() + update_from + 1, move_data.end());
  }

  if (update_from < packed_maps.size() - 1) {
    packed_maps.erase(packed_maps.begin() + update_from + 1, packed_maps.end());
  }

  move_data_t result     = move_data.back();
  bool        map_loaded = false;
  uint32_t    count      = 0;
  while (update_from < input.size()) { // only update anything after the last input change
    input_data_t* input_data = &(input[update_from]);
    if (input_data->needs_packed) {
      if (update_from < packed_maps.size()) { // do we have a packed map for this tick
        auto& packed_map = packed_maps[update_from];
        if (packed_map) {
          if (!map_loaded) { // is this map valid
            map_loaded = true;

            prediction_copy_t copy(PC_EVERYTHING, cheat::local_player, PC_DATA_NORMAL,
                                   &packed_map->map, PC_DATA_PACKED);
            copy.transfer_data("", cheat::local_player->entity_index(),
                               cheat::local_player->get_data_map());
          } else { // generate a map for this
            packed_map->map = {};
            prediction_copy_t copy(PC_EVERYTHING, &packed_map->map, PC_DATA_PACKED,
                                   cheat::local_player, PC_DATA_NORMAL);
            copy.transfer_data("", cheat::local_player->entity_index(),
                               cheat::local_player->get_data_map());
          }
        }
      } else { // push a new map
        auto& packed_map =
            this->packed_maps.emplace_back(std::make_unique<packed_map_holder>());
        prediction_copy_t copy(PC_EVERYTHING, &packed_map->map, PC_DATA_PACKED,
                               cheat::local_player, PC_DATA_NORMAL);

        copy.transfer_data("", cheat::local_player->entity_index(),
                           cheat::local_player->get_data_map());
      }
    } else {
      packed_maps.push_back(nullptr);
    }

    draw_data.emplace_back(cheat::local_player->flags());
    setup_move_data_input(&result, input_data); // setup move data for input
    move_data.push_back(result);
    process_movement(&result); // predict player movement
    update_from++;             // advance the tick we're looking at
    count++;
  }

  cheat::local_player->current_cmd() = nullptr;
}

void generate_input_strafe_towards(input_data_t& input_data, segment_t* segment) {
  input_data.yaw_change_type  = yaw_type::toward;
  input_data.yaw              = segment->yaw;
  input_data.accel_percentage = segment->strafe_towards_data.accel_percentage * 0.01f;
}

void generate_input_strafe_side(input_data_t& input_data, segment_t* segment) {
  input_data.yaw_change_type = yaw_type::relative;
  input_data.yaw             = segment->strafe_side_data.turn_rate;
  switch (segment->direction) {
    case left:
      input_data.side_move = -400.f;
      break;

    case right:
      input_data.side_move = 400.f;
      input_data.yaw *= -1.f;
      break;
  }
}

void generate_input_strafe_accel(input_data_t& input_data, segment_t* segment) {
  input_data.yaw_change_type  = yaw_type::relative_accel;
  input_data.accel_percentage = segment->strafe_accel_data.accel_percentage * 0.01f;
  input_data.turn_accel       = segment->strafe_accel_data.turn_accel_rate;
  input_data.turn_rate        = segment->strafe_accel_data.starting_turn_rate;
  switch (segment->direction) {
    case left:
      input_data.side_move = -400.f;
      break;

    case right:
      input_data.side_move = 400.f;
      break;
  }
}

void generate_input_pre_strafe(input_data_t& input_data, segment_t* segment) {
  input_data.yaw_change_type = yaw_type::pre_strafe;
  input_data.direction       = segment->direction;
}

void generate_input_manual(input_data_t& input_data, segment_t* segment) {
  input_data.yaw_change_type = yaw_type::set;
  input_data.yaw             = segment->yaw;
  input_data.pitch           = segment->manual_data.pitch;
  auto& manual_data          = segment->manual_data;
  if (manual_data.move_forward)
    input_data.forward_move = 400.f;
  if (manual_data.move_backward)
    input_data.forward_move = -400.f;
  if (manual_data.move_left)
    input_data.side_move = -400.f;
  if (manual_data.move_right)
    input_data.side_move = 400.f;
}

void tas_t::generate_input_for_segment(segment_t* segment, int& tick) {
  int   frame_count = segment->frames.size();
  float frame_div   = 0.1f;
  bool  first       = true;
  for (auto& frame : segment->frames) {
    input_data_t input_data;
    switch (segment->move_type) {
      case strafe_side:
        generate_input_strafe_side(input_data, segment);
        break;
      case strafe_towards:
        generate_input_strafe_towards(input_data, segment);
        break;
      case pre_strafe:
        generate_input_pre_strafe(input_data, segment);
        break;
      case manual:
        generate_input_manual(input_data, segment);
        break;
      case strafe_accel:
        generate_input_strafe_accel(input_data, segment);
        break;
    }
    if (frame.crouch)
      input_data.buttons |= IN_DUCK;
    input_data.bhop         = segment->bunnyhop;
    input_data.needs_packed = first;
    first                   = false;
    input.push_back(input_data);
    tick++;
  }
}

void tas_t::generate_input() {
  if (!setup)
    return;

  int tick = 0;
  {
    std::unique_lock lock(mutex);
    needs_update = false;
    for (auto& segment : segments) {
      if (properties.updated) {
        segment.updated    = true;
        properties.updated = false;
      }
      if (segment.updated || needs_update) {
        segment.start_tick = tick;
        if (!needs_update) {
          update_from = tick;
        }
        if (segment.updated)
          needs_update = true;

        if (!input.empty() && input.size() >= tick)
          input.erase(input.begin() + tick, input.end());

        generate_input_for_segment(&segment, tick);
        segment.end_tick = tick;
        segment.updated  = false;
      } else
        tick = segment.end_tick;
      segment.abs_selected_frame = segment.selected_frame + segment.start_tick;
    }
  }
}

void tas_t::playback() {
  if (start_playback) {
    in_playback    = true;
    playback_tick  = 0;
    start_playback = false;
  }

  if (input.empty() || playback_tick >= input.size()) {
    in_playback = false;
    return;
  }

  if (!in_playback)
    return;

  auto data = &move_data[playback_tick];
  setup_command_input(cheat::cmd, data);
  playback_tick++;

  if (set_playback_angles) {
    cheat::interfaces.engine_client->set_view_angles(cheat::cmd->view_angles);
  }
}

bool draw_hull(vector3_t origin, vector3_t mins, vector3_t maxs, float& max_x,
               float& min_y) noexcept {
  std::array<glm::vec3, 8> screen_corners{};

  for (size_t i{}; i < screen_corners.size(); ++i) {
    // Fetch a corner in world space.
    vector3_t corner_pos = {i & 1 ? maxs.x : mins.x, i & 2 ? maxs.y : mins.y,
                            i & 4 ? maxs.z : mins.z};

    // Bring the bbox into world space.
    corner_pos += origin;

    if (!math::world_to_screen(corner_pos, screen_corners[i])) {
      return false;
    }

    max_x = glm::max(screen_corners[i].x, max_x);
    if (min_y < 0)
      min_y = screen_corners[i].y;
    else
      min_y = glm::min(screen_corners[i].y, min_y);
  }

  if (max_x == -FLT_MAX || min_y == FLT_MAX) {
    return false;
  }

  constexpr auto color = IM_COL32(212, 209, 51, 255);

  cheat::render.tas_draw_cmds.line(screen_corners[0], screen_corners[2], color);
  cheat::render.tas_draw_cmds.line(screen_corners[0], screen_corners[4], color);
  cheat::render.tas_draw_cmds.line(screen_corners[2], screen_corners[6], color);
  cheat::render.tas_draw_cmds.line(screen_corners[6], screen_corners[4], color);

  cheat::render.tas_draw_cmds.line(screen_corners[0], screen_corners[1], color);
  cheat::render.tas_draw_cmds.line(screen_corners[2], screen_corners[3], color);
  cheat::render.tas_draw_cmds.line(screen_corners[4], screen_corners[5], color);
  cheat::render.tas_draw_cmds.line(screen_corners[6], screen_corners[7], color);

  cheat::render.tas_draw_cmds.line(screen_corners[1], screen_corners[3], color);
  cheat::render.tas_draw_cmds.line(screen_corners[1], screen_corners[5], color);
  cheat::render.tas_draw_cmds.line(screen_corners[3], screen_corners[7], color);
  cheat::render.tas_draw_cmds.line(screen_corners[7], screen_corners[5], color);

  return true;
}

// TODO: Move these.
struct teleport_trigger_t {
  vector3_t origin{};
  vector3_t mins{}, maxs{};
  vector3_t tp_pos{};
};

std::vector<teleport_trigger_t> triggers{};

void tas_t::on_create_move() {
  // TODO: Add error checking and improve this.
  auto str_to_vec3 = [](std::string_view str) noexcept {
    auto pos = str.data();
    auto end = pos + str.size();

    vector3_t res{};

    for (std::uint8_t i{}; i < 3; ++i) {
      float value{};
      auto  conv = std::from_chars(pos, end, value);
      if (conv.ec == std::errc{}) {
        res[i] = value;
      }

      pos = conv.ptr + 1;
    }

    return res;
  };

  static bool once{};
  if (!once && cheat::interfaces.bsp_data) {
    once = true;

    // Yes, I know I can just use the `map_entitystring` member inside the bsp
    // data, but I cannot be bothered to reverse engineer `CDiscardableArray`
    // right now.
    auto map_ents_str = cheat::interfaces.engine_client->get_map_entities_string();

    auto map_ents_res = kv::parse(map_ents_str);

    if (!map_ents_res) {
      printf("Map entity parsing failed: %s\n", map_ents_res.error().c_str());
    } else {
      auto& map_ents = map_ents_res.value();

      for (auto&& map_ent : map_ents) {
        // Find teleport entities.
        if (!map_ent.kvs.contains("classname")) {
          continue;
        }

        if (map_ent.kvs["classname"].value != "trigger_teleport") {
          continue;
        }

        // Find target origin.
        if (!map_ent.kvs.contains("target")) {
          printf("`trigger_teleport` has no target.\n");
          continue;
        }

        auto target_origin_res = [&map_ents,
                                  &str_to_vec3](std::string_view target_name) noexcept
            -> std::expected<vector3_t, std::string_view> {
          for (auto&& map_ent : map_ents) {
            // Find teleport destination entities.
            if (!map_ent.kvs.contains("classname")) {
              continue;
            }

            if (map_ent.kvs["classname"].value != "info_teleport_destination") {
              continue;
            }

            if (!map_ent.kvs.contains("targetname") ||
                map_ent.kvs["targetname"].value != target_name) {
              continue;
            }

            if (!map_ent.kvs.contains("origin")) {
              return std::unexpected{"No origin"};
            }

            return str_to_vec3(map_ent.kvs["origin"].value);
          }

          return std::unexpected{"Not found"};
        }(map_ent.kvs["target"].value);

        if (!target_origin_res) {
          printf("Couldn't find target origin (%s)\n", target_origin_res.error().data());
          continue;
        }

        // Lookup inline model.
        if (!map_ent.kvs.contains("model")) {
          printf("`trigger_teleport` has no model.\n");
          continue;
        }

        auto& inline_model_name = map_ent.kvs["model"].value;

        if (inline_model_name[0] != '*') {
          printf("`trigger_teleport` has an invalid inline model name.\n");
          continue;
        }

        std::int32_t model_idx{};
        auto         model_idx_conv =
            std::from_chars(inline_model_name.data() + 1,
                            inline_model_name.data() + inline_model_name.size(), model_idx);

        if (model_idx_conv.ec != std::errc{} || model_idx < 1 ||
            model_idx >= cheat::interfaces.bsp_data->numcmodels) {
          printf("`trigger_teleport` has a bad model index.\n");
          continue;
        }

        if (!map_ent.kvs.contains("origin")) {
          printf("`trigger_teleport` has no origin.\n");
          continue;
        }

        auto& model = cheat::interfaces.bsp_data->map_cmodels[model_idx];

        triggers.emplace_back(str_to_vec3(map_ent.kvs["origin"].value), model.mins, model.maxs,
                              target_origin_res.value());
      }
    }
  }

  setup_prediction(cheat::cmd);
  generate_input();
  handle_prediction();
  playback();
  draw();
}

void tas_t::draw_frame(move_data_t* move_data, draw_data_t* draw_data) noexcept {

  auto* game_rules   = *cheat::interfaces.game_rules;
  auto* view_vectors = game_rules->get_view_vectors();

  vector3_t mins, maxs;
  if (!!(draw_data->flags & FL_DUCKING)) {
    mins = view_vectors->duck_hull_min;
    maxs = view_vectors->duck_hull_max;
  } else {
    mins = view_vectors->hull_min;
    maxs = view_vectors->hull_max;
  }
  float max_x = -FLT_MAX;
  float min_y = FLT_MAX;
  if (draw_hull(move_data->abs_origin, mins, maxs, max_x, min_y)) {
    vector3_t velocity = move_data->velocity;

    if (glm::length(velocity) <= 1e-6f) { // Near zero.
      velocity = {};
    }

    std::string xyz_speed = std::format("Speed: {:.3f}", glm::length(velocity));
    std::string xy_speed  = std::format("Speed (2D): {:.3f}", glm::length(velocity.xy()));

    cheat::render.tas_draw_cmds.text(cheat::render.tahoma_bold_13, {max_x + 5, min_y},
                                     IM_COL32_WHITE, IM_COL32_BLACK, xyz_speed);
    cheat::render.tas_draw_cmds.text(cheat::render.tahoma_bold_13, {max_x + 5, min_y + 15},
                                     IM_COL32_WHITE, IM_COL32_BLACK, xy_speed);
  }
}

void tas_t::draw_segment(segment_t* segment, bool selected) noexcept {
  if (segment->end_tick >= move_data.size())
    return;
  auto color = IM_COL32(255, 255, 255, 255);
  if (selected) {
    vector3_t start_velocity = move_data[segment->start_tick].velocity;
    vector3_t end_velocity   = move_data[segment->end_tick - 1].velocity;
    if (glm::length(end_velocity.xy()) > glm::length(start_velocity.xy()))
      color = IM_COL32(100, 255, 100, 255);
    else
      color = IM_COL32(255, 100, 100, 255);
  }

  vector3_t last_screen{};
  bool      first = true;
  for (auto i = segment->start_tick; i <= segment->end_tick; i++) {
    vector3_t screen;
    if (!math::world_to_screen(move_data[i].abs_origin, screen))
      continue;

    if (first) {
      first       = false;
      last_screen = screen;
      continue;
    }
    cheat::render.tas_draw_cmds.line(last_screen, screen, color, 2);
    last_screen = screen;
  }
}

void tas_t::draw() noexcept {
  static decltype(segments) tmp_segments{};
  static std::int32_t       tmp_selected_segment{};

  if (move_data.empty())
    return;

  {
    std::shared_lock _{mutex};

    if (segments.empty())
      return;

    tmp_segments         = segments;
    tmp_selected_segment = selected_segment;
  }

  int max_drawn_segments = max_segments_rendered;
  int max_segment        = static_cast<int>(tmp_segments.size()) - 1;
  int min_segment        = 0;

  if (max_drawn_segments > 0) {
    max_drawn_segments--;
    max_segment = static_cast<int>(glm::round(max_drawn_segments / 2));
    max_drawn_segments -= max_segment;
    max_segment += tmp_selected_segment;
    min_segment = tmp_selected_segment - max_drawn_segments;

    max_segment = glm::min<int>(static_cast<int>(tmp_segments.size() - 1), max_segment);
    min_segment = glm::max(0, min_segment);
  }

  bool      first = true;
  vector3_t last;
  vector3_t segment_starting_velocity{};

  // for (auto& trigger : triggers) {
  //   float max_x = -FLT_MAX;
  //   float min_y = FLT_MAX;
  //   draw_hull(trigger.origin, trigger.mins, trigger.maxs, max_x, min_y);
  //
  //   vector3_t screen_pos_start{}, screen_pos_end{};
  //   math::world_to_screen(trigger.origin, screen_pos_start);
  //   math::world_to_screen(trigger.tp_pos, screen_pos_end);
  //
  //   cheat::render.tas_draw_cmds.line({screen_pos_start.x,
  //   screen_pos_start.y},
  //                                    {screen_pos_end.x, screen_pos_end.y},
  //                                    IM_COL32(0, 255, 0, 255), 4);
  // }

  for (size_t i = min_segment; i <= max_segment; ++i) {
    auto* current_segment = &tmp_segments[i];
    if (current_segment->updated)
      return;
    draw_segment(current_segment, i == tmp_selected_segment);
  }

  auto* selected_segment = &tmp_segments[tmp_selected_segment];
  if (!selected_segment->updated && selected_segment->end_tick < move_data.size())
    draw_frame(&move_data[selected_segment->abs_selected_frame],
               &draw_data[selected_segment->abs_selected_frame]);

  cheat::render.tas_draw_cmds.flush();
}
