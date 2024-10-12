#pragma once

#include "cvar.h"
#include "app_system.h"

class console_display_func_t {
public:
  virtual void color_print(const color& clr, const char* p_message) = 0;
  virtual void print(const char* message)                           = 0;
  virtual void d_print(const char* message)                         = 0;
};

class cvar_query_t : public app_system_t {
public:
  virtual bool are_con_vars_linkable(const convar_t* child, const convar_t* parent) = 0;
};

class cvar_t : public app_system_t {
public:
  virtual cvar_dll_identifier_t allocate_dll_identifier()                                = 0;
  virtual void                  register_con_command(con_command_base_t* command_base)   = 0;
  virtual void                  unregister_con_command(con_command_base_t* command_base) = 0;
  virtual void                  unregister_con_commands(cvar_dll_identifier_t id)        = 0;
  virtual const char*           get_command_line_value(const char* variable_name)        = 0;
  virtual con_command_base_t*   find_command_base(const char* name)                      = 0;
  virtual const con_command_base_t* find_command_base(const char* name) const            = 0;
  virtual convar_t*                 find_var(const char* var_name)                       = 0;
  virtual const convar_t*           find_var(const char* var_name) const                 = 0;
  virtual con_command_t*            find_command(const char* name)                       = 0;
  virtual const con_command_t*      find_command(const char* name) const                 = 0;
  virtual con_command_base_t*       get_commands(void)                                   = 0;
  virtual const con_command_base_t* get_commands(void) const                             = 0;
  virtual void install_global_change_callback(change_callback_t callback)                = 0;
  virtual void remove_global_change_callback(change_callback_t callback)                 = 0;
  virtual void call_global_change_callbacks(convar_t* var, const char* old_string,
                                            float fold_value)                            = 0;
  virtual void install_console_display_func(console_display_func_t* display_func)        = 0;
  virtual void remove_console_display_func(console_display_func_t* display_func)         = 0;
  virtual void console_color_printf(const color& clr, const char* format, ...) const     = 0;
  virtual void console_printf(const char* format, ...) const                             = 0;
  virtual void console_d_printf(const char* format, ...) const                           = 0;
  virtual void revert_flagged_convars(int flag)                                          = 0;
  virtual void install_cvar_query(cvar_query_t* p_query)                                 = 0;
  virtual bool is_material_thread_set_allowed() const                                    = 0;
  virtual void queue_material_thread_set_value(convar_t* cvar, const char* value)        = 0;
  virtual void queue_material_thread_set_value(convar_t* cvar, int value)                = 0;
  virtual void queue_material_thread_set_value(convar_t* cvar, float value)              = 0;
  virtual bool has_queued_material_thread_convar_sets() const                            = 0;
  virtual int  process_queued_material_thread_con_var_sets()                             = 0;

protected:
  class cvar_iterator_internal_t;

public:
  class iterator {
  public:
    inline iterator(cvar_t* icvar);
    inline ~iterator(void);
    inline void                set_first(void);
    inline void                next(void);
    inline bool                is_valid(void);
    inline con_command_base_t* get(void);

  private:
    cvar_iterator_internal_t* m_iter;
  };

protected:
  class cvar_iterator_internal_t {
  public:
    virtual ~cvar_iterator_internal_t() {}
    virtual void                set_first(void) = 0;
    virtual void                next(void)      = 0;
    virtual bool                is_valid(void)  = 0;
    virtual con_command_base_t* get(void)       = 0;
  };
  virtual cvar_iterator_internal_t* factory_internal_iterator(void) = 0;
  friend class iterator;
};
