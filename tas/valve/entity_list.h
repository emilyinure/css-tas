#pragma once

#include "valve/entities/base_handle.h"

class client_networkable;
class client_entity;

class entity_list_t {
public:
  virtual client_networkable* get_client_networkable(int entity_number)         = 0;
  virtual client_networkable* get_client_networkable_from_handle(base_handle_t) = 0;
  virtual client_networkable* get_client_unknown_from_handle(base_handle_t)     = 0;

private:
  virtual client_entity* get_client_entity(int entity_number)         = 0;
  virtual client_entity* get_client_entity_from_handle(base_handle_t) = 0;

public:
  virtual int entity_count(bool exclude_non_networkable) = 0;
  virtual int get_highest_entity_index(void)             = 0;

private:
  virtual void set_max_entities(int max_entities) = 0;
  virtual int  get_max_entities()                 = 0;

public:
  template <typename T> T* get_entity(int entity_number) {
    return reinterpret_cast<T*>(get_client_entity(entity_number));
  }
  template <typename T> T* get_entity(base_handle_t handle) {
    return reinterpret_cast<T*>(get_client_entity_from_handle(handle));
  }
};
