#include "compression_handler.h"

#include "core/offsets/offsets.h"
#include "glm/gtc/noise.inl"
#include "valve/entities/player.h"

auto get_new(float old_val, float new_val, float tolerance) noexcept {
  return glm::abs(new_val - old_val) <= tolerance ? old_val : new_val;
}

glm::vec3 get_new(const vector3_t& old_val, const vector3_t& new_val,
                  float tolerance) noexcept {
  return {get_new(old_val.x, new_val.x, tolerance), get_new(old_val.y, new_val.y, tolerance),
          get_new(old_val.z, new_val.z, tolerance)};
}

void shared_netvar::post_update(player_t* player, int idx) {
  m_value[idx] = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + m_offset);
  *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + m_offset) =
      get_new(m_old_value[idx], m_value[idx], m_tolerance);
}

void shared_netvar::pre_update(player_t* player, int idx) {
  m_old_value[idx] =
      *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + m_offset);
}

void managed_vec::pre_update(player_t* player, int idx) {
  m_old_value[idx] = *reinterpret_cast<vector3_t*>(reinterpret_cast<std::uintptr_t>(player) +
                                                   m_proxied_offset);
}

void managed_vec::post_update(player_t* player, int idx) {
  m_value[idx] =
      *reinterpret_cast<vector3_t*>(reinterpret_cast<std::uintptr_t>(player) + m_offset);
  *reinterpret_cast<vector3_t*>(reinterpret_cast<std::uintptr_t>(player) + m_offset) =
      get_new(m_old_value[idx], m_value[idx], m_tolerance);
}

void prediction_netvar_manager::pre_update(player_t* player, int idx) {
  has_saved[idx] = true;

  for (auto&& var : vars) {
    var->pre_update(player, idx);
  }
}

void prediction_netvar_manager::post_update(player_t* player, int idx) {
  if (!has_saved[idx]) {
    return;
  }

  for (auto&& var : vars) {
    var->post_update(player, idx);
  }
}

constinit float EQUAL_EPSILON = 1e-3;

bool CloseEnough(float a, float b, float epsilon = EQUAL_EPSILON) {
  return glm::abs(a - b) <= epsilon;
}

float AssignRangeMultiplier(int nBits, double range) {
  unsigned long iHighValue;
  if (nBits == 32) {
    iHighValue = 0xFFFFFFFE;
  } else {
    iHighValue = (1 << (unsigned long)nBits) - 1;
  }

  auto fHighLowMul = static_cast<float>(iHighValue / range);
  if (CloseEnough(static_cast<float>(range), 0)) {
    fHighLowMul = static_cast<float>(iHighValue);
  }

  // If the precision is messing us up, then adjust it so it won't.
  if ((unsigned long)(fHighLowMul * range) > iHighValue ||
      fHighLowMul * range > (double)iHighValue) {
    // Squeeze it down smaller and smaller until it's going to produce an
    // integer in the valid range when given the highest value.
    float multipliers[] = {0.9999f, 0.99f, 0.9f, 0.8f, 0.7f};
    int   i;
    for (i = 0; i < ARRAYSIZE(multipliers); i++) {
      fHighLowMul = (float)(iHighValue / range) * multipliers[i];
      if ((unsigned long)(fHighLowMul * range) > iHighValue ||
          fHighLowMul * range > (double)iHighValue) {
      } else {
        break;
      }
    }

    if (i == ARRAYSIZE(multipliers)) {
      // Doh! We seem to be unable to represent this range.
      return 0;
    }
  }

  return fHighLowMul;
}

void prediction_netvar_manager::init(datamap_t* map) {
  if (std::exchange(initalized, true)) {
    return;
  }

  float val = 1.0f / AssignRangeMultiplier(17, 4096.0f - -4096.0f);
  vars.push_back(std::make_unique<shared_netvar>(offsets.base_player.fall_velocity, val,
                                                 "m_flFallVelocity"));
  val = 1.0f / AssignRangeMultiplier(14, 1400.0f);
  vars.push_back(std::make_unique<shared_netvar>(offsets.base_player.stamina, val, "stamina"));

  val = 1.0f / AssignRangeMultiplier(20, 2000.0f);
  printf("%d\n", offsets.base_player.base_velocity);
  printf("%f\n", val);
  vars.push_back(std::make_unique<managed_vec>(offsets.base_player.base_velocity, val,
                                               "m_vecBaseVelocity"));
}

prediction_netvar_manager::~prediction_netvar_manager() {
  for (auto&& i : vars) {
    i.reset();
  }

  vars.clear();
}
