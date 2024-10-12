#pragma once

typedef void* (*instantiate_interface_fn)();
typedef void* (*create_interface_fn)(const char* name, int return_code);

class interface_reg_t {
public:
  instantiate_interface_fn create_fn;
  const char*              name;
  interface_reg_t*         next;
};
