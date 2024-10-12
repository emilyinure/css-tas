#pragma once

#include "Windows.h"
#include "library/math.h"
#include <memory>
#include <vector>

class datamap_t;
class player_t;

class managed_netvar {
public:
  virtual ~managed_netvar() = default;

  virtual void pre_update(player_t* player, int idx)  = 0;
  virtual void post_update(player_t* player, int idx) = 0;

  uintptr_t m_offset{};
};

class shared_netvar : public managed_netvar {
public:
  shared_netvar(uintptr_t offset, float tolerance, const char* name) : m_tolerance(tolerance) {
    m_offset = offset;
    m_name   = _strdup(name);
  }

  void pre_update(player_t* player, int idx) override;
  void post_update(player_t* player, int idx) override;

  float m_value[128]{};
  float m_old_value[128]{};
  float m_tolerance{};
  char* m_name{};
};

class managed_vec : public managed_netvar {
public:
  managed_vec(uintptr_t offset, float tolerance, const char* name, bool coord = false)
      : m_tolerance(tolerance), m_coord(coord) {
    m_offset         = offset;
    m_proxied_offset = offset;
    m_name           = _strdup(name);
  }

  managed_vec(uintptr_t offset, uintptr_t proxied_offset, float tolerance, const char* name,
              bool coord = false)
      : m_tolerance(tolerance), m_coord(coord) {
    m_offset         = offset;
    m_proxied_offset = proxied_offset;
    m_name           = _strdup(name);
  }

  void pre_update(player_t* player, int idx) override;
  void post_update(player_t* player, int idx) override;

  vector3_t m_value[128]{};
  vector3_t m_old_value[128]{};
  float     m_tolerance{};
  char*     m_name{};
  bool      m_coord{};
  uintptr_t m_proxied_offset{};
};

class prediction_netvar_manager {
public:
  prediction_netvar_manager() = default;
  virtual ~prediction_netvar_manager();
  void reset() {
    for (size_t i{}; i < 128; i++)
      has_saved[i] = false;
  }
  void pre_update(player_t*, int);
  void post_update(player_t*, int);
  void init(datamap_t* map);

  bool                                         has_saved[128]{};
  bool                                         initalized{};
  bool                                         setup_vars{};
  std::vector<std::unique_ptr<managed_netvar>> vars{};
  std::vector<managed_netvar*>                 weapon_vars;
  bool                                         called_once{};
};
