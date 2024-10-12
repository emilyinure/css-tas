#include "offsets.h"
#include "cheat.h"
#include "core/netvars/netvars.h"
#include "library/hash.h"
#include "library/pe.h"

#include <numeric>

bool offsets_t::get_offsets() {
  cheat::debug.print("base weapon offsets initialized", console_color_red);

  pe::module_t client_dll;
  if (!pe::get_module("client.dll", client_dll))
    return false;
  functions.set_collision_bounds =
      client_dll.find_pattern_in_memory("55 8B EC 83 EC 2C 53 8B 5D 0C 56 57 8B 7D 08");
  functions.save_data     = client_dll.find_pattern_in_memory("55 8B EC 83 EC 7C 53 57");
  functions.restore_data  = client_dll.find_pattern_in_memory("55 8B EC 83 EC 7C 53 56 57");
  functions.transfer_data = client_dll.find_pattern_in_memory("55 8B EC FF 05 ?? ?? ?? ?? 53");
  functions.prediction_copy_init =
      client_dll.find_pattern_in_memory("55 8B EC 8B 45 08 89 01 8B 45 0C 89 41 04 8B 45 14");
  functions.write_usercmd = client_dll.find_pattern_in_memory("55 8B EC 8B 45 10 83 EC 08");
  cheat::debug.print("client module offsets initialized", console_color_red);
  datamaps = client_dll.find_all_pattern_in_memory(
      "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3 CC");
  netvars_t netvars;
  cheat::debug.print("initalizing netvars");

  base_entity.origin = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_vecOrigin"));
  // base_entity.network_origin =
  //     netvars.get_netvar_offset(HASH("DT_BaseEntity"),
  //     HASH("m_vecNetworkOrigin"));
  base_entity.mins   = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_vecMins"));
  base_entity.maxs   = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_vecMaxs"));

  base_entity.move_type = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("movetype"));
  base_entity.simulation_time =
      netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_flSimulationTime"));
  base_entity.owner = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_hOwnerEntity"));
  base_entity.team_id = netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_iTeamNum"));
  base_entity.ang_rotation =
      netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_angRotation"));
  base_entity.hitbox_set = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_nHitboxSet"));
  base_entity.collision_prop =
      netvars.get_netvar_offset(HASH("DT_BaseEntity"), HASH("m_Collision"));

  cheat::debug.print("base entity offsets initialized", console_color_red);

  base_player.view_offset =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_vecViewOffset[0]"));
  base_player.flags      = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_fFlags"));
  base_player.tick_base  = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_nTickBase"));
  base_player.life_state = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_lifeState"));
  base_player.health     = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_iHealth"));
  base_player.velocity =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_vecVelocity[0]"));
  base_player.max_speed =
      netvars.get_netvar_offset(HASH("DT_BasePlayer"), HASH("m_flMaxspeed"));
  base_player.eye_angles =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_angEyeAngles[0]"));
  base_player.active_weapon =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_hActiveWeapon"));
  base_player.contraint_entity =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_hConstraintEntity"));
  base_player.next_attack =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_flNextAttack"));
  base_player.stamina = netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_flStamina"));
  base_player.fall_velocity =
      netvars.get_netvar_offset(HASH("DT_CSPlayer"), HASH("m_flFallVelocity"));
  base_player.base_velocity =
      netvars.get_netvar_offset(HASH("C_BasePlayer"), HASH("m_vecBaseVelocity"));
  base_player.surface_friction =
      netvars.get_netvar_offset(HASH("C_BasePlayer"), HASH("m_surfaceFriction"));

  cheat::debug.print("base player offsets initialized", console_color_red);

  base_weapon.next_primary_attack =
      netvars.get_netvar_offset(HASH("DT_BaseCombatWeapon"), HASH("m_flNextPrimaryAttack"));
  base_weapon.next_secondary_attack =
      netvars.get_netvar_offset(HASH("DT_BaseCombatWeapon"), HASH("m_flNextSecondaryAttack"));
  base_weapon.clip1 = netvars.get_netvar_offset(HASH("DT_BaseCombatWeapon"), HASH("m_iClip1"));
  base_weapon.clip2 = netvars.get_netvar_offset(HASH("DT_BaseCombatWeapon"), HASH("m_iClip2"));

  return true;
}
