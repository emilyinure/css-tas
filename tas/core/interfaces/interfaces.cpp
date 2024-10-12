#include "interfaces.h"
#include "cheat.h"
#include "library/hash.h"
#include "library/pe.h"

#include <chrono>
#include <cstdint>
#include <thread>

bool interfaces_t::collect_interfaces() {
  pe::module_t server_browser;
  while (!pe::get_module("serverbrowser.dll", server_browser))
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

  cheat::debug.print("init client.dll");
  pe::module_t client_dll;
  if (!pe::get_module("client.dll", client_dll))
    return false;

  cheat::debug.print("init engine.dll");
  pe::module_t engine_dll;
  if (!pe::get_module("engine.dll", engine_dll))
    return false;

  cheat::debug.print("init vstdlib.dll");
  pe::module_t vstdlib_dll;
  if (!pe::get_module("vstdlib.dll", vstdlib_dll))
    return false;

  cheat::debug.print("init shaderapidx9.dll");
  pe::module_t shaderapidx9_dll;
  if (!pe::get_module("shaderapidx9.dll", shaderapidx9_dll))
    return false;

  // GTFGCClientSystem() func
  //  [actual address in first opcode] E8 ? ? ? ? 8B C8 E8 ? ? ? ? EB 17

  d3d9_device = **reinterpret_cast<IDirect3DDevice9***>(
      shaderapidx9_dll.find_pattern_in_memory("A1 ? ? ? ? 50 8B 08 FF 51 0C") + 0x1);

  cheat::debug.print("init client017");
  base_client = client_dll.get_interface<base_client_t*>(HASH("VClient017"));
  cheat::debug.print("init EntityList");
  entity_list = client_dll.get_interface<entity_list_t*>(HASH("VClientEntityList003"));
  cheat::debug.print("init clinet prediction");
  prediction = client_dll.get_interface<prediction_manager_t*>(HASH("VClientPrediction001"));
  cheat::debug.print("init gamemovement");
  game_movement = client_dll.get_interface<game_movement_t*>(HASH("GameMovement001"));
  cheat::debug.print("init cvar");
  console = vstdlib_dll.get_interface<cvar_t*>(HASH("VEngineCvar004"));

  cheat::debug.print("init client mode");
  client_mode = **reinterpret_cast<void***>((*(std::uintptr_t**)base_client)[10] + 5);

  cheat::debug.print("init input");
  input = **reinterpret_cast<cinput_t***>(
      utils::get_virtual_function<std::uintptr_t>(base_client, 5) + 18);

  cheat::debug.print("init game rules");
  game_rules = *reinterpret_cast<ccsgamerules_t***>(
      client_dll.find_pattern_in_memory("8B 0D ? ? ? ? 57 8B 01 74 16") + 2);

  cheat::debug.print("init engine client");
  engine_client = engine_dll.get_interface<engine_client_t*>(HASH("VEngineClient013"));
  cheat::debug.print("init render view");
  render_view = engine_dll.get_interface<render_view_t*>(HASH("VEngineRenderView014"));
  cheat::debug.print("init vgui");
  engine_vgui = engine_dll.get_interface<void*>(HASH("VEngineVGui001"));
  cheat::debug.print("init trace client");
  engine_trace = engine_dll.get_interface<engine_trace_t*>(HASH("EngineTraceClient003"));
  cheat::debug.print("init global vars");
  global_vars = *reinterpret_cast<global_vars_t**>(
      engine_dll.find_pattern_in_memory("A1 ? ? ? ? 8B 11 68") + 0x8);
  cheat::debug.print("init model info client");
  model_info = engine_dll.get_interface<model_info_t*>(HASH("VModelInfoClient006"));

  cheat::debug.print("init bsp data");
  bsp_data = *reinterpret_cast<collision_bsp_data_t**>(
      engine_dll.find_pattern_in_memory("68 ? ? ? ? 56 E8 ? ? ? ? E8") + 1);

  return true;
}
