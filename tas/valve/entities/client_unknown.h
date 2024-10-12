#pragma once

#include "valve/entities/handle_entity.h"

class collideable_t;
class client_renderable_t;
class client_networkable_t;
class client_entity_t;
class client_thinkable_t;
class base_entity_t;

class client_unknown_t : public handle_entity_t {
public:
  virtual collideable_t*        collideable()           = 0;
  virtual client_networkable_t* get_networkable()       = 0;
  virtual client_renderable_t*  get_client_renderable() = 0;
  virtual client_entity_t*      get_client_entity()     = 0;
  virtual base_entity_t*        get_base_entity()       = 0;
  virtual client_thinkable_t*   get_client_thinkable()  = 0;
};
