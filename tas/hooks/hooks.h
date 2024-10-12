#include "hook.h"
#include "safetyhook.hpp"

class usercmd_t;

class hooks_t {
private:
  struct minhook_original_functions {
    typedef bool(__stdcall* create_move_fn)(float, usercmd_t*);
    create_move_fn create_move;
  } minhook_original_functions;

  void setup_detour_hooks() noexcept;
  void setup_vtable_hooks() noexcept;

public:
  hook_t        client_mode;
  hook_t        base_client;
  hook_t        prediction;
  hook_t        d3d9_device;
  hook_t        engine_vgui;
  SafetyHookVmt input_hook{};
  SafetyHookVm  write_usercmd_delta_to_buffer_hook{};
  SafetyHookVm  get_usercmd_hook{};

  void initialize() noexcept;
  void detach() noexcept;
};
