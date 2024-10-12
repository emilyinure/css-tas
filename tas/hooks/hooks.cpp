#include "cheat.h"
#include "library/pe.h"
#include "core/offsets/offsets.h"
#include "core/settings/settings.h"
#include "valve/base_client.h"
#include "valve/entities/player.h"
#include "valve/prediction_copy.h"
#include "valve/input.h"
#include "valve/bf_write.h"

extern long ImGui_ImplWin32_WndProcHandler(HWND, unsigned, unsigned, long);

void __thiscall create_move(base_client_t* thisptr, std::int32_t sequence, float sample,
                            std::int32_t active) noexcept {
  cheat::hooks.base_client.get_virtual_function<decltype(&create_move)>(21)(thisptr, sequence,
                                                                            sample, active);

  cheat::on_create_move(cheat::interfaces.input->get_usercmd(sequence));
}

void __thiscall paint(void* thisptr, std::int32_t mode) noexcept {
  cheat::get_view_matrix();

  cheat::hooks.engine_vgui.get_virtual_function<decltype(&paint)>(13)(thisptr, mode);
}

void __thiscall frame_stage_notify(base_client_t*       thisptr,
                                   client_frame_stage_t stage) noexcept {
  cheat::pre_frame_stage_notify(stage);

  cheat::hooks.base_client.get_virtual_function<decltype(&frame_stage_notify)>(35)(thisptr,
                                                                                   stage);

  cheat::post_frame_stage_notify(stage);
}

void __thiscall run_command(prediction_manager_t* thisptr, player_t* player, usercmd_t* cmd,
                            move_helper_t* move_helper) {
  cheat::hooks.prediction.get_virtual_function<decltype(&run_command)>(17)(thisptr, player, cmd,
                                                                           move_helper);
  cheat::interfaces.move_helper = move_helper;

  if (cheat::local_player != nullptr) {
    cheat::compression_manager.init(cheat::local_player->get_data_map());
    cheat::compression_manager.pre_update(cheat::local_player, cmd->command_number % 90);
  }
}

HRESULT __stdcall present(IDirect3DDevice9* device, RECT* source_rect, RECT* dest_rect,
                          HWND dest_window_override, RGNDATA* dirty_region) noexcept {
  cheat::on_present();

  return cheat::hooks.d3d9_device.get_virtual_function<decltype(&present)>(17)(
      device, source_rect, dest_rect, dest_window_override, dirty_region);
}

HRESULT __stdcall reset(IDirect3DDevice9*      device,
                        D3DPRESENT_PARAMETERS* present_parameters) noexcept {
  cheat::render.detach();

  auto result = cheat::hooks.d3d9_device.get_virtual_function<decltype(&reset)>(16)(
      device, present_parameters);

  cheat::get_window_handle();
  cheat::render.initialize(); // this is really bad..

  return result;
}

void __thiscall override_view(void* thisptr, view_setup_t* view) noexcept {
  cheat::hooks.client_mode.get_virtual_function<decltype(&override_view)>(16)(thisptr, view);
}

void __thiscall pre_entity_packet_received(prediction_manager_t* thisptr,
                                           std::int32_t          commands_acknowledged,
                                           std::int32_t current_world_update_packet) noexcept {
  cheat::get_local_player();

  if (cheat::local_player && commands_acknowledged > 0) {
    cheat::local_player->restore_data("PreEntityPacketReceived", commands_acknowledged - 1,
                                      PC_EVERYTHING);
  }

  cheat::hooks.prediction.get_virtual_function<decltype(&pre_entity_packet_received)>(4)(
      thisptr, commands_acknowledged, current_world_update_packet);
}

void __thiscall post_network_data_received(prediction_manager_t* thisptr,
                                           std::int32_t commands_acknowledged) noexcept {
  cheat::get_local_player();

  if (cheat::local_player && commands_acknowledged > 0) {
    cheat::compression_manager.init(cheat::local_player->get_data_map());
    cheat::compression_manager.post_update(
        cheat::local_player,
        (cheat::interfaces.prediction->server_commands_acknowledged + commands_acknowledged) %
            90);
    // cheat::local_player->save_data("post_network_data_received", -1, PC_EVERYTHING);
  }

  cheat::hooks.prediction.get_virtual_function<decltype(&post_network_data_received)>(6)(
      thisptr, commands_acknowledged);
}

void hooks_t::setup_detour_hooks() noexcept {}

class hooked_cinput_t : public cinput_t {
public:
  bool hooked_write_usercmd_delta_to_buffer(bf_write_t* buf, std::int32_t from, std::int32_t to,
                                            bool isnewcommand) noexcept {
    // NOTE: Recreating this function to get rid of `CUserCmd` checksums entirely.
    usercmd_t nullcmd{}, *f{&nullcmd}, *t{&nullcmd};

    if (from != -1) {
      if (auto from_cmd = get_usercmd(from)) {
        f = from_cmd;
      }
    }

    if (auto to_cmd = get_usercmd(to)) {
      t = to_cmd;
    }

    static auto write_usercmd =
        reinterpret_cast<void(__cdecl*)(bf_write_t*, usercmd_t*, usercmd_t*)>(
            offsets.functions.write_usercmd);

    write_usercmd(buf, t, f);

    return !buf->IsOverflowed();
  }

  usercmd_t* hooked_get_usercmd(std::int32_t sequence_number) noexcept {
    // NOTE: Recreating this function to bypass the check inside it, so we can forcefully set
    // command numbers.
    return &cmds[sequence_number % MULTIPLAYER_BACKUP];
  }
};

void hooks_t::setup_vtable_hooks() noexcept {
  input_hook = safetyhook::create_vmt(cheat::interfaces.input);

  get_usercmd_hook = safetyhook::create_vm(input_hook, 8, &hooked_cinput_t::hooked_get_usercmd);

  write_usercmd_delta_to_buffer_hook = safetyhook::create_vm(
      input_hook, 5, &hooked_cinput_t::hooked_write_usercmd_delta_to_buffer);

  client_mode.initialize(reinterpret_cast<uintptr_t>(cheat::interfaces.client_mode));

  // client_mode.setup_hook(21, reinterpret_cast<uintptr_t>(create_move));

  client_mode.setup_hook(16, reinterpret_cast<uintptr_t>(override_view));

  base_client.initialize(reinterpret_cast<uintptr_t>(cheat::interfaces.base_client));

  base_client.setup_hook(35, reinterpret_cast<uintptr_t>(frame_stage_notify));
  base_client.setup_hook(21, reinterpret_cast<uintptr_t>(create_move));

  d3d9_device.initialize(reinterpret_cast<uintptr_t>(cheat::interfaces.d3d9_device));

  d3d9_device.setup_hook(17, reinterpret_cast<uintptr_t>(present));

  d3d9_device.setup_hook(16, reinterpret_cast<uintptr_t>(reset));

  engine_vgui.initialize(reinterpret_cast<uintptr_t>(cheat::interfaces.engine_vgui));

  engine_vgui.setup_hook(13, reinterpret_cast<uintptr_t>(paint));

  prediction.initialize(reinterpret_cast<uintptr_t>(cheat::interfaces.prediction));

  prediction.setup_hook(17, reinterpret_cast<uintptr_t>(run_command));
  prediction.setup_hook(6, reinterpret_cast<uintptr_t>(post_network_data_received));
  prediction.setup_hook(4, reinterpret_cast<uintptr_t>(pre_entity_packet_received));
}

void hooks_t::initialize() noexcept {
  setup_vtable_hooks();
  setup_detour_hooks();
}

void hooks_t::detach() noexcept {
  client_mode.undo_hooks();
  base_client.undo_hooks();
  d3d9_device.undo_hooks();
  engine_vgui.undo_hooks();
  prediction.undo_hooks();
  input_hook = {};
}
