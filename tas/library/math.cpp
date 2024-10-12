#include "math.h"
#include "cheat.h"

bool math::world_to_screen(const vector3_t& origin, vector3_t& screen_position) {
  view_matrix_t& matrix = cheat::view_matrix;

  const float w = matrix[3][0] * origin.x + matrix[3][1] * origin.y + matrix[3][2] * origin.z +
                  matrix[3][3];

  if (w <= 1e-6f)
    return false;

  auto half_screen_size = cheat::render.screen_size / 2;

  screen_position.x = static_cast<float>(half_screen_size.x);
  screen_position.y = static_cast<float>(half_screen_size.y);

  screen_position.x *= 1 + (matrix[0][0] * origin.x + matrix[0][1] * origin.y +
                            matrix[0][2] * origin.z + matrix[0][3]) /
                               w;
  screen_position.y *= 1 - (matrix[1][0] * origin.x + matrix[1][1] * origin.y +
                            matrix[1][2] * origin.z + matrix[1][3]) /
                               w;
  screen_position.z = 0.0f;

  return true;
}

float math::ticks_to_time(int ticks) {
  return (cheat::interfaces.global_vars->tick_interval * (float)(ticks));
}

int math::time_to_ticks(float time) {
  return static_cast<int>(0.5f + (float)(time) / cheat::interfaces.global_vars->tick_interval);
}
