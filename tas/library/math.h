#pragma once

#include <glm/geometric.hpp>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <algorithm>
#include <numbers>

using vector2_t      = glm::vec2;
using vector3_t      = glm::vec3;
using vector3_a_t    = glm::aligned_vec3;
using matrix_3x4_a_t = glm::aligned_mat3x4;
using matrix_3x4_t   = glm::mat3x4;
using view_matrix_t  = glm::mat<4, 4, float, glm::defaultp>;

namespace math {
  constexpr double PI   = 3.14159265358979323846;
  constexpr double PI_2 = 1.57079632679489661923;
  constexpr float  radian_to_degrees(const double x) { return x * (180.f / PI); }

  constexpr float degrees_to_radian(const float x) { return x * (PI / 180.f); }

  inline float remap_val_clamped(float val, float A, float B, float C, float D) {
    if (A == B)
      return val >= B ? D : C;
    float cVal = (val - A) / (B - A);
    cVal       = std::clamp<float>(cVal, 0.0f, 1.0f);

    return C + (D - C) * cVal;
  }

  inline double normalize_rad(double a) {
    a = fmod(a, std::numbers::pi_v<double> * 2);

    if (a >= std::numbers::pi_v<double>) {
      a -= 2 * std::numbers::pi_v<double>;
    } else if (a < -std::numbers::pi_v<double>) {
      a += 2 * std::numbers::pi_v<double>;
    }

    return a;
  }

  inline vector3_t vector_transform(matrix_3x4_t& matrix, vector3_t offset) {
    return vector3_t{glm::dot(offset, vector3_t(matrix[0])) + matrix[0][3],
                     glm::dot(offset, vector3_t(matrix[1])) + matrix[1][3],
                     glm::dot(offset, vector3_t(matrix[2])) + matrix[2][3]};
  }

  float ticks_to_time(int ticks);
  int   time_to_ticks(float time);

  static void sin_cos(float r, float* s, float* c) {
    *s = sin(r);
    *c = cos(r);
  }

  inline float normalize_angle(float angle) { return std::remainder(angle, 360.0f); }

  inline void angle_vector(const vector3_t& from, vector3_t& forward) {
    const auto sy = sin(degrees_to_radian(from.y));
    const auto sp = sin(degrees_to_radian(from.x));
    const auto cy = cos(degrees_to_radian(from.y));
    const auto cp = cos(degrees_to_radian(from.x));

    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
  }

  inline void angle_vectors(const vector3_t& from, vector3_t* forward, vector3_t* right,
                            vector3_t* up) {
    float sp, sy, sr, cp, cy, cr;

    sin_cos(degrees_to_radian(from.x), &sp, &cp);
    sin_cos(degrees_to_radian(from.y), &sy, &cy);
    sin_cos(degrees_to_radian(from.z), &sr, &cr);

    if (forward) {
      forward->x = cp * cy;
      forward->y = cp * sy;
      forward->z = -sp;
    }

    if (right) {
      right->x = -1 * sr * sp * cy + -1 * cr * -sy;
      right->y = -1 * sr * sp * sy + -1 * cr * cy;
      right->z = -1 * sr * cp;
    }

    if (up) {
      up->x = cr * sp * cy + -sr * -sy;
      up->y = cr * sp * sy + -sr * cy;
      up->z = cr * cp;
    }
  }

  inline float length_2d(vector3_t vector) {
    return std::sqrtf(std::pow(vector.x, 2) + std::pow(vector.y, 2));
  }

  constexpr void vector_angle(const vector3_t& from, vector3_t to, vector3_t& result) {
    to -= from;
    if (to.y == 0.0f && to.x == 0.0f) {
      result.x = (to.z > 0.0f) ? 270.0f : 90.0f;
      result.y = 0.0f;
    } else {
      result.x = radian_to_degrees(atan2(-to.z, length_2d(to)));
      result.y = radian_to_degrees(atan2(to.y, to.x));
    }

    result.z = 0.0f;
  }

  inline float normalize_in_place(vector3_t& vec) {
    const float hyp = glm::length(vec);
    if (hyp > 0.f)
      vec *= (1.f / hyp);
    else
      vec = vector3_t(0, 0, 0);
    return hyp;
  }

  inline float delta_to_angle(vector3_t direction, vector3_t to_ang) {
    direction *= 1.f / glm::length(direction);
    vector3_t forward;
    angle_vector(to_ang, forward);

    return radian_to_degrees(acos(glm::dot(forward, direction)));
  }

  bool world_to_screen(const vector3_t& origin, vector3_t& screen_position);

} // namespace math
