#pragma once

class client_class_t;
class client_unknown_t;

class client_networkable_t {
public:
  virtual client_unknown_t* get_i_client_unknown()               = 0;
  virtual void              release()                            = 0;
  virtual client_class_t*   get_client_class()                   = 0;
  virtual void              notify_should_transmit()             = 0;
  virtual void              on_pre_data_changed()                = 0;
  virtual void              on_data_changed()                    = 0;
  virtual void              pre_data_update()                    = 0;
  virtual void              post_data_update()                   = 0;
  virtual bool              is_dormant()                         = 0;
  virtual int               entity_index()                       = 0;
  virtual void              receive_message()                    = 0;
  virtual void*             get_data_table_base_ptr()            = 0;
  virtual void              set_destroyed_on_recreate_entities() = 0;
  virtual void              on_data_unchanged_in_pvs()           = 0;
};
