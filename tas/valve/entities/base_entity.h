#pragma once

#include "client_entity.h"
#include "base_handle.h"

#include "../../core/offsets/offsets.h"
#include "library/screen_position.h"
#include "library/utils.h"

class player_t;
class weapon_t;

class collision_property_t {
public:
  void set_collision_bounds(const vector3_t& mins, const vector3_t& maxs) noexcept;
};

class base_entity_t : public client_entity_t {
  friend player_t;
  friend weapon_t;
  template <typename T> inline T get_ptr_at_offset(uint32_t offset) noexcept {
    return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(this) + offset);
  }

  template <typename T> inline T& get_value_at_offset(uint32_t offset) noexcept {
    return *get_ptr_at_offset<T*>(offset);
  }

#define netvar_value_func(type, name, offset)                                                  \
  type& name() noexcept { return get_value_at_offset<type>(offset); }
#define get_ptr_value_func(type, name, offset)                                                 \
  type name() noexcept { return get_ptr_at_offset<type>(offset); }

public:
  netvar_value_func(vector3_t, origin, offsets.base_entity.origin);
  netvar_value_func(float, simulation_time, offsets.base_entity.simulation_time);
  netvar_value_func(vector3_t, velocity, offsets.base_player.velocity);
  netvar_value_func(vector3_t, mins, offsets.base_entity.mins);
  netvar_value_func(vector3_t, maxs, offsets.base_entity.maxs);
  netvar_value_func(int, collision_group, 896);
  netvar_value_func(int, team_number, offsets.base_entity.team_id);
  netvar_value_func(int, hitbox_set, offsets.base_entity.hitbox_set);
  netvar_value_func(collision_property_t, collision, offsets.base_entity.collision_prop);

  collision_property_t* collision_prop() noexcept;
  void set_collision_bounds(const vector3_t& mins, const vector3_t& maxs) noexcept;
  bool get_bounding_box(area_t& out, vector3_t& origin) noexcept;
  bool should_collide(int group, int mask) noexcept;
};
