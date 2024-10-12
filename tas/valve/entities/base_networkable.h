#pragma once

#include <cstdint>

class client_class_t;

class base_networkable_t {
  virtual void*           get_client_unknown() = 0;
  virtual void            release()            = 0;
  virtual client_class_t* client_class()       = 0;
};
