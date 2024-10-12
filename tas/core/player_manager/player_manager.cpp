#include "player_manager.h"
#include "valve/entities/player.h"
#include "cheat.h"
#include "library/math.h"
#include "valve/client_class_id.h"
#include "valve/coord_size.h"

player_tick_record_t::player_tick_record_t(player_tick_manager_t* manager) {
  this->manager                    = manager;
  player_manager_t* player_manager = manager->manager;

  origin                = player_manager->origin;
  abs_origin            = player_manager->abs_origin;
  velocity              = player_manager->velocity;
  eye_angles            = player_manager->eye_angles;
  unstuck_origin        = player_manager->unstuck_origin;
  ground_input_wish_dir = player_manager->ground_input_wish_dir;
  air_input_wish_dir    = player_manager->air_input_wish_dir;

  simulation_time = player_manager->simulation_time;
  flags           = player_manager->player->flags();

  bones_setup = manager->manager->player->get_client_renderable()->setup_bones(
      bones.data(), 128, 0, simulation_time);
}

void player_tick_manager_t::update(player_manager_t* manager) {
  this->manager               = manager;
  player_tick_record_t record = player_tick_record_t(this);
  records.push_front(record);
  while (records.size() > 256)
    records.pop_back();
}

void player_manager_t::post_network_update() {
  check_targeting();
  if (player->life_state() != life_state_t::life_alive)
    return;

  this->abs_origin = player->get_abs_origin();

  if (player->simulation_time() <= simulation_time)
    return;

  float delta_sim_time = player->simulation_time() - simulation_time;

  predicted_origin = vector3_t(0, 0, 0);

  simulation_time = player->simulation_time();

  vector3_t updated_origin = player->origin();

  bool should_override = prediction_successful;
  if (prediction_successful) {
    vector3_t delta = (predicted_origin - updated_origin);
    if (abs(delta.x) > DIST_EPSILON)
      should_override = false;
    else if (abs(delta.y) > DIST_EPSILON)
      should_override = false;
  }

  needs_input_prediction = true;
  vector3_t last_origin  = origin;
  if (should_override) {
    origin                 = {predicted_origin.x, predicted_origin.y, updated_origin.z};
    needs_input_prediction = false;
  } else {

    origin = updated_origin;
  }

  velocity = (origin - last_origin) * (1.f / delta_sim_time);
  if (player->flags() & FL_ONGROUND)
    velocity.z = 0.f;

  eye_angles = player->eye_angles();

  tick_manager.update(this);
}

void player_manager_t::check_targeting() {
  auto networkable = player->get_networkable();
  is_target        = !networkable->is_dormant() &&
              player->team_number() != cheat::local_player->team_number() &&
              player->life_state() == life_alive;
}
bool player_list_t::initialize() { return true; }
void player_list_t::post_network_update() {
  for (size_t i = 1; i < cheat::interfaces.global_vars->max_clients; i++) {
    player_t* player = cheat::interfaces.entity_list->get_entity<player_t>(i);
    player_lists[i - 1].update(player);
    if (player)
      player_lists[i - 1].post_network_update();
  }
}
