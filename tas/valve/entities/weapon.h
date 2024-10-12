#pragma once

#include "library/utils.h"
#include "valve/entities/base_entity.h"

class weapon_t : public base_entity_t {
public:
  netvar_value_func(int, item_id, offsets.base_weapon.item_definition_index);
  netvar_value_func(float, next_primary_attack, offsets.base_weapon.next_primary_attack);
  netvar_value_func(float, next_secondary_attack, offsets.base_weapon.next_secondary_attack);
  netvar_value_func(float, charge_begin_time, offsets.base_weapon.charge_begin_time);
  int weapon_id() {
    return utils::get_virtual_function<int(__thiscall*)(void*)>(this, 381)(this);
  }
};
