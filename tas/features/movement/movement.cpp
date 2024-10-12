#include "movement.h"
#include "valve/entities/player.h"
#include "valve/usercmd.h"
#include "cheat.h"
#include "core/settings/settings.h"
#include "library/math.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

void movement_t::on_create_move() {
  move_angles = cheat::cmd->view_angles;

  auto_strafe();

  bunny_hop();
}

double movement_t::max_accel_theta(float speed, double wishspeed, bool on_ground) {
  static auto air_accel    = cheat::interfaces.console->find_var("sv_airaccelerate");
  static auto acceleration = cheat::interfaces.console->find_var("sv_accelerate");
  double accel = on_ground ? acceleration->value.float_value : air_accel->value.float_value;
  double accelspeed = accel * wishspeed * cheat::local_player->surface_friction() *
                      cheat::interfaces.global_vars->tick_interval;

  if (accelspeed <= 0.0)
    return math::PI;

  double wishspeed_capped = on_ground ? wishspeed : 30;
  double tmp              = wishspeed_capped - accelspeed;
  if (tmp <= 0.0)
    return math::PI_2;

  if (tmp < speed)
    return glm::acos(tmp / speed);

  return 0.0;
}
double movement_t::const_speed_theta(float speed, double wishspeed, bool on_ground) {
  static auto air_accel    = cheat::interfaces.console->find_var("sv_airaccelerate");
  static auto acceleration = cheat::interfaces.console->find_var("sv_accelerate");
  double      gamma1       = cheat::interfaces.global_vars->tick_interval * wishspeed *
                  cheat::local_player->surface_friction();
  double numer, denom;

  if (!on_ground) {
    gamma1 *= air_accel->value.float_value;
    if (gamma1 <= 60) {
      numer = -gamma1;
      denom = 2 * speed;
    } else {
      numer = -30;
      denom = speed;
    }
  } else {
    // will implement this if needed, id rather avoid it
    return max_accel_theta(speed, wishspeed, on_ground);
    // gamma1 *= acceleration->value.float_value;
    // double sqrdiff = player.SpeedBeforeFriction * player.SpeedBeforeFriction - speedsqr;
    // double tmp     = sqrdiff / gamma1;
    // if (tmp + gamma1 <= 2 * wishspeed) {
    //   numer = tmp - gamma1;
    //   denom = 2 * std::sqrt(speedsqr);
    // } else if (gamma1 > wishspeed && wishspeed * wishspeed >= sqrdiff) {
    //   numer = -std::sqrt(wishspeed * wishspeed - sqrdiff);
    //   denom = std::sqrt(speedsqr);
    // } else
    //   return max_accel_theta(speed, wishspeed, 1.f, on_ground);
  }
  // if we need it ill implement this for ground, i dont see a use.

  if (denom < std::fabs(numer))
    return max_accel_theta(speed, wishspeed, on_ground);

  return std::acos(numer / denom);
}

double movement_t::max_deccel_theta(float speed, double wishspeed, bool on_ground) {
  static auto air_accel    = cheat::interfaces.console->find_var("sv_airaccelerate");
  static auto acceleration = cheat::interfaces.console->find_var("sv_accelerate");
  double accel = on_ground ? acceleration->value.float_value : air_accel->value.float_value;
  double accelspeed = accel * wishspeed * cheat::interfaces.global_vars->tick_interval;

  if (accelspeed < 0.0) {
    double wishspeed_capped = on_ground ? wishspeed : 30;

    return glm::acos(wishspeed_capped / speed);
  }

  return math::PI;
}

double movement_t::max_accel_into_yaw_theta(const double& speed, const double& vel_yaw,
                                            const double& yaw, const double& wish_speed,
                                            bool on_ground) {
  const double theta = max_accel_theta(speed, wish_speed, on_ground);

  if (theta == 0.0 || theta == std::numbers::pi_v<double>) {
    return math::normalize_rad(yaw - vel_yaw + theta);
  }

  return copysign(theta, math::normalize_rad(yaw - vel_yaw));
}

void movement_t::auto_strafe() {
  if (!settings::movement::auto_strafe)
    return;
  if (cheat::local_player->flags() & FL_ONGROUND)
    return;

  vector3_t velocity = cheat::local_player->velocity();

  velocity.z = 0;

  float speed = glm::length(velocity);

  vector3_t direction;
  direction.x = (fabsf(cheat::cmd->side_move) < 1) && (fabsf(cheat::cmd->forward_move) < 1);
  direction.x += (cheat::cmd->forward_move > 0) - (cheat::cmd->forward_move < 0);
  direction.y = (cheat::cmd->side_move < 0) - (cheat::cmd->side_move > 0);

  double view_rad    = math::normalize_rad(math::degrees_to_radian(move_angles.y) +
                                           atan2(direction.y, direction.x));
  double vel_yaw_rad = atan2(velocity.y, velocity.x);

  double theta = max_accel_into_yaw_theta(speed, vel_yaw_rad, view_rad, 400.f);

  cheat::cmd->side_move    = (theta > 0) * 400.f + (theta < 0) * -400.f;
  cheat::cmd->forward_move = 0.f;

  double buttons_phi = -atan2(cheat::cmd->side_move, cheat::cmd->forward_move);
  move_angles.y      = math::normalize_angle(
      math::radian_to_degrees(math::normalize_rad(vel_yaw_rad - buttons_phi + theta)));
}

void movement_t::bunny_hop() {
  if (!settings::movement::bunny_hop)
    return;
  if (cheat::local_player->flags() & FL_ONGROUND)
    return;

  cheat::cmd->buttons &= ~IN_JUMP;
}
void movement_t::correct_movement() {
  vector3_t wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

  const vector3_t movedata{cheat::cmd->forward_move, cheat::cmd->side_move,
                           cheat::cmd->up_move};

  math::angle_vectors(move_angles, &wish_forward, &wish_right, &wish_up);
  math::angle_vectors(cheat::cmd->view_angles, &cmd_forward, &cmd_right, &cmd_up);

  const double v8 =
                   sqrtf(wish_forward[0] * wish_forward[0] + wish_forward[1] * wish_forward[1]),
               v10 = sqrtf(wish_right[0] * wish_right[0] + wish_right[1] * wish_right[1]),
               v12 = sqrtf(wish_up[2] * wish_up[2]);

  const vector3_t wish_forward_norm(1.0f / v8 * wish_forward[0], 1.0f / v8 * wish_forward[1],
                                    0.f);

  const vector3_t wish_right_norm(1.0f / v10 * wish_right[0], 1.0f / v10 * wish_right[1], 0.f);
  const vector3_t wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up[2]);

  const double v14 = sqrtf(cmd_forward[0] * cmd_forward[0] + cmd_forward[1] * cmd_forward[1]),
               v16 = sqrtf(cmd_right[0] * cmd_right[0] + cmd_right[1] * cmd_right[1]),
               v18 = sqrtf(cmd_up[2] * cmd_up[2]);

  const vector3_t cmd_forward_norm(1.0f / v14 * cmd_forward[0], 1.0f / v14 * cmd_forward[1],
                                   1.0f / v14 * 0.0f),
      cmd_right_norm(1.0f / v16 * cmd_right[0], 1.0f / v16 * cmd_right[1], 1.0f / v16 * 0.0f),
      cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up[2]);

  const double v22 = wish_forward_norm[0] * movedata[0],
               v26 = wish_forward_norm[1] * movedata[0],
               v28 = wish_forward_norm[2] * movedata[0], v24 = wish_right_norm[0] * movedata[1],
               v23 = wish_right_norm[1] * movedata[1], v25 = wish_right_norm[2] * movedata[1],
               v30 = wish_up_norm[0] * movedata[2], v27 = wish_up_norm[2] * movedata[2],
               v29 = wish_up_norm[1] * movedata[2];

  vector3_t correct_movement{
      cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 +
          (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) +
          (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27),

      cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 +
          (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) +
          (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27),

      cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 +
          (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) +
          (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27)};

  vector3_t new_correct_movement = correct_movement;
  if (fabsf(new_correct_movement.x) > 400.f) {
    float percent = std::abs(400.f / new_correct_movement.x);
    new_correct_movement.x *= percent;
    new_correct_movement.y *= percent;
  }

  if (fabsf(new_correct_movement.y) > 400.f) {
    float percent = std::abs(400.f / new_correct_movement.y);
    new_correct_movement.x *= percent;
    new_correct_movement.y *= percent;
  }

  cheat::cmd->forward_move = std::clamp<float>(new_correct_movement[0], -400.f, 400.f);
  cheat::cmd->side_move    = std::clamp<float>(new_correct_movement[1], -400.f, 400.f);
  cheat::cmd->up_move      = std::clamp<float>(new_correct_movement[2], -320.f, 320.f);
}
