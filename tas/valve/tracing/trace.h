#pragma once

#include "../../library/math.h"

#define DISPSURF_FLAG_SURFACE   (1 << 0)
#define DISPSURF_FLAG_WALKABLE  (1 << 1)
#define DISPSURF_FLAG_BUILDABLE (1 << 2)
#define DISPSURF_FLAG_SURFPROP1 (1 << 3)
#define DISPSURF_FLAG_SURFPROP2 (1 << 4)

class base_entity_t;

struct plane_t {
  vector3_t normal;
  float     dist;
  std::byte type;
  std::byte sign_bits;
  std::byte pad[2];
};

struct surface_t {
  const char*    name;
  short          surface_props;
  unsigned short flags;
};

struct trace_t {
  vector3_t      start{};
  vector3_t      end{};
  plane_t        plane{};
  float          fraction            = 0;
  int            contents            = 0;
  unsigned short disp_flags          = 0;
  bool           all_solid           = 0;
  bool           start_solid         = 0;
  float          fraction_left_solid = 0.f;
  surface_t      surface{};
  int            hitgroup     = 0;
  short          physics_bone = 0;
  base_entity_t* entity       = nullptr;
  int            hitbox       = 0;

  bool did_hit() const { return fraction < 1.f || all_solid || start_solid; }
};
