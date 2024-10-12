#pragma once

#include "color.h"

// forward decl.
class con_command_base_t;
typedef int cvar_dll_identifier_t;
class command_t;
class i_convar_t;

typedef void (*change_callback_t)(i_convar_t* var, const char* old_value, float f_old_value);
typedef void (*command_callback_void_t)(void);
typedef void (*command_callback_t)(const command_t& command);
typedef int (*command_completion_callback_t)(const char* partial, char commands[64][64]);

class con_command_base_accessor_t {
public:
  virtual bool register_con_command_base(con_command_base_t* var) = 0;
};

class i_convar {
public:
  virtual void        set_value(const char* p_value)       = 0;
  virtual void        set_value(float value)               = 0;
  virtual void        set_value(int value)                 = 0;
  virtual void        set_value(color value)               = 0;
  virtual const char* get_name(void) const                 = 0;
  virtual const char* get_base_name(void) const            = 0;
  virtual bool        is_flag_set(int n_flag) const        = 0;
  virtual int         get_split_screen_player_slot() const = 0;
};

class con_command_base_t {
public:
  virtual ~con_command_base_t(void);
  virtual bool                  is_command(void) const;
  virtual bool                  is_flag_set(int flag) const;
  virtual void                  add_flags(int flags);
  virtual void                  remove_flags(int flags);
  virtual int                   get_flags() const;
  virtual const char*           get_name(void) const;
  virtual const char*           get_help_text(void) const;
  virtual bool                  is_registered(void) const;
  virtual cvar_dll_identifier_t get_dll_identifier() const;
  virtual void create(const char* name, const char* help_string = 0, int flags = 0);
  virtual void init();

  con_command_base_t* next;
  bool                registered;
  const char*         name;
  const char*         help_string;
  int                 flags;
};

class command_t {
public:
  command_t();
  command_t(int n_arg_c, const char** pp_arg_v);

private:
  enum {
    command_max_argc   = 64,
    command_max_length = 512,
  };

  int         m_argc;
  int         m_argv0_size;
  char        m_arg_s_buffer[command_max_length];
  char        m_argv_buffer[command_max_length];
  const char* m_argv[command_max_argc];
};

class command_callback_manager_t {
public:
  virtual void command_callback(const command_t& command) = 0;
};

class command_completion_callback_manager_t {
public:
  virtual int command_completion_callback(); // const char* partial, c_utl_vector<c_utl_string>&
                                             // commands) = 0;
};

class con_command_t : public con_command_base_t {
public:
  virtual ~con_command_t(void);
  virtual bool is_command(void) const;
  virtual int
  auto_complete_suggest() = 0; // const char* partial, c_utl_vector<c_utl_string>& commands);
  virtual bool can_auto_complete(void);
  virtual void dispatch(const command_t& command);

  union {
    command_callback_void_t     m_fn_command_callback_v1;
    command_callback_t          m_fn_command_callback;
    command_callback_manager_t* m_p_command_callback;
  };
  union {
    command_completion_callback_t          m_fn_completion_callback;
    command_completion_callback_manager_t* m_p_command_completion_callback;
  };

  bool m_has_completion_callback;
  bool m_using_new_command_callback;
  bool m_using_command_callback_interface;
};

class convar_t : public con_command_base_t, public i_convar {
public:
  typedef con_command_base_t base_class;
  virtual ~convar_t(void);
  virtual bool        is_flag_set(int flag) const;
  virtual const char* get_help_text(void) const;
  virtual bool        is_registered(void) const;
  virtual const char* get_name(void) const;
  virtual const char* get_base_name(void) const;
  virtual int         get_split_screen_player_slot() const;
  virtual void        add_flags(int flags);
  virtual int         get_flags() const;
  virtual bool        is_command(void) const;
  virtual void        set_value(const char* value);
  virtual void        set_value(float value);
  virtual void        set_value(int value);
  virtual void        set_value(color value);

  struct cv_value_t {
    char* string_value;
    int   string_length;
    float float_value;
    int   int_value;
  };

  virtual void internal_set_value(const char* value);
  virtual void internal_set_float_value(float f_new_value);
  virtual void internal_set_int_value(int n_value);
  virtual void internal_set_color_value(color value);
  virtual bool clamp_value(float& value);
  virtual void change_string_value(const char* temp_val, float fl_old_value);
  virtual void create(const char* name, const char* default_value, int flags = 0,
                      const char* help_string = 0, bool min = false, float f_min = 0.0,
                      bool b_max = false, float f_max = false, change_callback_t callback = 0);
  virtual void init();

  convar_t*   parent;
  const char* default_value;
  cv_value_t  value;
  bool        has_min;
  float       min_val;
  bool        has_max;
  float       max_val;
  // c_utl_vector<fn_change_callback_t> m_change_callbacks;
};
