#pragma once

class recv_proxy_data;
class recv_prop_t;
class recv_decoder_t;
class recv_table_t;

typedef void (*recv_var_proxy_fn)(const recv_proxy_data* data, void* strct, void* out);
typedef void (*array_length_proxy_fn)(void* strct, int object_id, int current_array_length);
typedef void (*data_table_recv_var_proxy_fn)(const recv_prop_t* prop, void** out, void* data,
                                             int object_id);

class recv_prop_t {
public:
  const char*                  var_name;
  int                          recv_type;
  int                          flags;
  int                          string_buffer_size;
  bool                         inside_array;
  const void*                  m_extra_data;
  recv_prop_t*                 array_prop;
  array_length_proxy_fn        array_length_proxy;
  recv_var_proxy_fn            proxy;
  data_table_recv_var_proxy_fn data_table_proxy;
  recv_table_t*                data_table;
  int                          offset;
  int                          element_stride;
  int                          num_of_elements;
  const char*                  parent_array_prop_name;
};

class recv_table_t {
public:
  recv_prop_t*    props;
  int             num_of_props;
  recv_decoder_t* decoder;
  const char*     net_table_name;

private:
  bool initialized;
  bool in_main_list;
};
