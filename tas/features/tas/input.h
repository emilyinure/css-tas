#pragma once

#include "library/math.h"

enum movement_direction_t : int { left, right };

enum class yaw_type { set, relative, relative_accel, toward, pre_strafe };

struct input_data_t {
  float                forward_move     = 0;
  float                side_move        = 0;
  float                up_move          = 0;
  float                yaw              = 0;
  float                pitch            = 0;
  int                  buttons          = 0;
  float                accel_percentage = 0;
  yaw_type             yaw_change_type  = yaw_type::set;
  bool                 bhop             = false;
  bool                 needs_packed     = false;
  movement_direction_t direction{left};
  float                turn_rate       = 0;
  float                turn_accel_rate = 0;
  float                turn_accel      = 0;
};

