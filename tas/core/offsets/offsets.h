#pragma once

#include <cstdint>
#include <vector>

struct offsets_t {
  bool get_offsets();

  struct {
    uint32_t origin, network_origin, team_id, health, life_state, owner, simulation_time,
        spawn_time, move_type, mins, maxs, ang_rotation, max_speed, hitbox_set, collision_prop;
  } base_entity;

  struct {
    uint32_t item_definition_index, clip1, clip2, next_primary_attack, next_secondary_attack,
        charge_begin_time;
  } base_weapon;

  struct {
    uint32_t view_offset, health, life_state, ground, velocity, eye_angles, taunt, flags,
        active_weapon, next_attack, max_speed, tick_base, class_id, shared, contraint_entity,
        base_velocity, fall_velocity, stamina, surface_friction;
  } base_player;

  struct {
    uint32_t save_data, restore_data, transfer_data, prediction_copy_init, write_usercmd,
        set_collision_bounds;
  } functions;

  std::vector<uint32_t> datamaps = {};
} inline offsets;
