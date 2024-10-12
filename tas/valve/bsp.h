#pragma once

#include "library/math.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <expected>

constexpr std::size_t MAX_QPATH = 96;

template <class T> class range_validated_array_t {
public:
  const auto& operator[](std::int32_t i) const noexcept { return array[i]; }

  T*           array;
  std::int32_t count;
};

// TODO: Move this.
namespace kv {
  struct KeyValues {
    std::string                                name{};
    std::string                                value{};
    std::unordered_map<std::string, KeyValues> kvs{};

    auto& operator[](const std::string& key) { return kvs[key]; }
  };

  // Parse a KeyValue.
  std::expected<std::vector<KeyValues>, std::string> parse(std::string_view str);
} // namespace kv

#pragma pack(push, 1)

class node_t {
public:
};

class phys_collide_t {
public:
};

class vcollide_t {
public:
  std::uint16_t    solid_count : 15;
  std::uint16_t    is_packed : 1;
  std::uint16_t    desc_size;
  phys_collide_t** solids; // VPhysicsSolids
  char*            key_values;
};

class model_t {
public:
  vector3_t    mins;
  vector3_t    maxs;
  vector3_t    origin;
  std::int32_t headnode;
  vcollide_t   vcollisionData;
};

static_assert(sizeof(model_t) == 52);

class collision_bsp_data_t {
public:
  node_t*                          map_rootnode;
  char                             map_name[MAX_QPATH];
  std::uint8_t                     pad_00[80];
  std::int32_t                     numcmodels;
  range_validated_array_t<model_t> map_cmodels;
};

static_assert(offsetof(collision_bsp_data_t, numcmodels) == 180);

#pragma pack(pop)