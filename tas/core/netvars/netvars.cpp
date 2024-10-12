#include "netvars.h"
#include "library/hash.h"
#include "core/debug/debug.h"
#include "valve/datamap.h"
#include "core/offsets/offsets.h"
#include "cheat.h"

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <cstdio>
#include <ranges>
#include <algorithm>
#include <vector>

netvars_t::netvars_t() {
  client_class_t* client_class = cheat::interfaces.base_client->get_all_classes();

  while (client_class) {
    recv_table_t* recv_table = client_class->recv_table;

    if (!recv_table) {
      client_class = client_class->next;
      continue;
    }

    const auto table_name = hash::hash_crc(recv_table->net_table_name);
    var_map_t  var_map;
    iterate_props(&var_map, recv_table);
    netvar_map.insert({table_name, var_map});

    client_class = client_class->next;
  }
  find_and_store_data_maps();
}

void netvars_t::store_data_map(uint32_t addr) {
  datamap_t*         map = nullptr;
  uint32_t           base, var;
  typedescription_t* entry = nullptr;
  addr                     = *(uint32_t*)(addr + 2);
  map                      = (datamap_t*)(addr - 4);

  if (!map || !map->dataNumFields || map->dataNumFields > 200 || !map->dataDesc ||
      !map->dataClassName)
    return;

  base = hash::hash_crc(map->dataClassName);

  for (size_t i = 0; i < map->dataNumFields; i++) {
    entry = &map->dataDesc[i];
    if (!entry->fieldName)
      continue;

    var                   = hash::hash_crc(entry->fieldName);
    netvar_map[base][var] = entry->fieldOffset[0];
  }
}

void netvars_t::find_and_store_data_maps() {
  if (offsets.datamaps.empty())
    return;
  for (auto& addr : offsets.datamaps) {
    store_data_map(addr);
  }
}

netvars_t::~netvars_t() { netvar_map.clear(); }

void netvars_t::iterate_props(var_map_t* map, recv_table_t* table, uint32_t offset) {
  if (!table)
    return;
  for (size_t i = 0; i < table->num_of_props; i++) {
    const auto& prop = table->props[i];
    if (prop.data_table && prop.num_of_elements)
      iterate_props(map, prop.data_table, offset + prop.offset);

    map->insert({hash::hash_crc(prop.var_name), offset + prop.offset});
  }
}

uintptr_t netvars_t::get_netvar_offset(uint32_t table_name, uint32_t var_name) {
  auto var_map = netvar_map.at(table_name);
  return var_map.at(var_name);
}

uintptr_t netvars_t::get_netvar_offset(const char* table_name, const char* var_name) {
  return get_netvar_offset(hash::hash_crc(table_name), hash::hash_crc(var_name));
}
