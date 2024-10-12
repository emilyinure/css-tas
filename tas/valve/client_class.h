#pragma once

#include "recv.h"

typedef void* (*create_client_class_fn)(int ent_num, int serial_num);
typedef void* (*create_event_fn)();

class client_class_t {
public:
  create_client_class_fn create;
  create_event_fn        create_event;
  const char*            network_name;
  recv_table_t*          recv_table;
  client_class_t*        next;
  int                    class_id;
};
