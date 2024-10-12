#include "base_entity.h"

void collision_property_t::set_collision_bounds(const vector3_t& mins,
                                                const vector3_t& maxs) noexcept {
  using set_collision_bounds_fn =
      void(__thiscall*)(collision_property_t*, const vector3_t&, const vector3_t&);
  static auto fn =
      reinterpret_cast<set_collision_bounds_fn>(offsets.functions.set_collision_bounds);

  fn(this, mins, maxs);
}

collision_property_t* base_entity_t::collision_prop() noexcept { return &collision(); }

void base_entity_t::set_collision_bounds(const vector3_t& mins,
                                         const vector3_t& maxs) noexcept {
  auto col = collision_prop();
  if (col == nullptr) {
    return;
  }

  col->set_collision_bounds(mins, maxs);
}

bool base_entity_t::get_bounding_box(area_t& out, vector3_t& origin) noexcept {
  vector3_t mins_screen = {}, maxs_screen = {}, origin_screen = {};

  if (!math::world_to_screen(this->mins() + origin, mins_screen) ||
      !math::world_to_screen(this->maxs() + origin, maxs_screen) ||
      !math::world_to_screen(origin, origin_screen))
    return false;

  out.h = abs(mins_screen.y - maxs_screen.y);
  out.y = std::min(maxs_screen.y, mins_screen.y);
  out.w = out.h / 2;
  out.x = origin_screen.x - (out.w / 2);
  return true;
}

bool base_entity_t::should_collide(int group, int mask) noexcept {
  return utils::get_virtual_function<bool(__thiscall*)(void*, int, int)>(this, 145)(this, group,
                                                                                    mask);
}