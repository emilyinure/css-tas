#pragma once

#include <stdint.h>
#include "../../library/math.h"
#include <glm/gtx/norm.hpp>
#include "mask_defines.h"
#include "trace_filter.h"

struct trace_t;

struct ray_t {
  vector3_a_t start;        // starting point, centered within the extents
  vector3_a_t delta;        // direction + length of the ray
  vector3_a_t start_offset; // Add this to m_Start to get the actual ray start
  vector3_a_t extents;      // Describes an axis aligned box extruded along a ray
  bool        is_ray;       // are the extents zero?
  bool        is_swept;     // is delta != 0?

  void initialize(const vector3_t& start, const vector3_t& end) {
    this->delta = end - start;

    this->is_swept = (glm::length(glm::vec2(this->delta)) != 0);

    this->extents = {0, 0, 0};
    this->is_ray  = true;

    this->start_offset = {0, 0, 0};
    this->start        = start;
  }

  void initialize(vector3_t const& start, vector3_t const& end, vector3_t const& mins,
                  vector3_t const& maxs) {
    this->delta = end - start;

    this->is_swept = (glm::length(this->delta) != 0);

    this->extents = (maxs - mins);
    this->extents *= 0.5f;
    this->is_ray = (glm::length(glm::vec2(this->extents)) < 1e-6);

    // Offset m_Start to be in the center of the box...
    vector3_t offset = (mins + maxs) * 0.5f;

    this->start        = (start + offset);
    this->start_offset = offset;
    this->start_offset *= -1.0f;
  }

  ray_t() {}

  ray_t(vector3_t vec_start, vector3_t vec_end) { initialize(vec_start, vec_end); }

  ray_t(vector3_t vec_start, vector3_t vec_end, vector3_t min, vector3_t max) {
    initialize(vec_start, vec_end, min, max);
  }
};

class engine_trace_t {
public:
  void trace_ray(const ray_t& ray, uint32_t mask, i_trace_filter_t* filter, trace_t* trace) {
    typedef void(__thiscall * TraceRayFn)(void*, const ray_t&, unsigned int, i_trace_filter_t*,
                                          trace_t*);
    return utils::get_virtual_function<TraceRayFn>(this, 4)(this, ray, mask, filter, trace);
  }
};
