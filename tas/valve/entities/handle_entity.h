#pragma once

class handle_entity_t {
public:
  virtual ~handle_entity_t()    = 0;
  virtual void set_ref_handle() = 0;
  virtual void get_ref_handle() = 0;
};
