#pragma once

#include "library/math.h"

class movement_t {
public:
  static double max_accel_theta(float speed, double wishspeed, bool on_ground = false);
  static double max_deccel_theta(float speed, double wishspeed, bool on_ground = false);
  static double const_speed_theta(float speed, double wishspeed, bool on_ground = false);

  static double max_accel_into_yaw_theta(const double& speed, const double& vel_yaw,
                                         const double& yaw, const double& wish_speed,
                                         bool on_ground = false);

  void      on_create_move();
  void      auto_strafe();
  void      bunny_hop();
  void      correct_movement();
  vector3_t move_angles;
};
