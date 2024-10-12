#include "base_entity.h"
#include "library/math.h"
#include "valve/entities/base_handle.h"
#include <stdint.h>
#include "../datamap.h"

class usercmd_t;

#define FL_ONGROUND   (1 << 0)
#define FL_DUCKING    (1 << 1)
#define FL_WATERJUMP  (1 << 2)
#define FL_ONTRAIN    (1 << 3)
#define FL_INRAIN     (1 << 4)
#define FL_FROZEN     (1 << 5)
#define FL_ATCONTROLS (1 << 6)
#define FL_CLIENT     (1 << 7)
#define FL_FAKECLIENT (1 << 8)
#define FL_INWATER    (1 << 9)

enum team_id : int { TEAM_NONE, TEAM_SPEC, TEAM_RED, TEAM_BLUE };

enum collision_group_t : int {
  COLLISION_GROUP_NONE = 0,
  COLLISION_GROUP_DEBRIS,             // Collides with nothing but world and static stuff
  COLLISION_GROUP_DEBRIS_TRIGGER,     // Same as debris, but hits triggers
  COLLISION_GROUP_INTERACTIVE_DEBRIS, // Collides with everything except other interactive
                                      // debris or debris
  COLLISION_GROUP_INTERACTIVE, // Collides with everything except interactive debris or debris
  COLLISION_GROUP_PLAYER,
  COLLISION_GROUP_BREAKABLE_GLASS,
  COLLISION_GROUP_VEHICLE,
  COLLISION_GROUP_PLAYER_MOVEMENT, // For HL2, same as Collision_Group_Player, for
  // TF2, this filters out other players and CBaseObjects
  COLLISION_GROUP_NPC,           // Generic NPC group
  COLLISION_GROUP_IN_VEHICLE,    // for any entity inside a vehicle
  COLLISION_GROUP_WEAPON,        // for any weapons that need collision detection
  COLLISION_GROUP_VEHICLE_CLIP,  // vehicle clip brush to restrict vehicle movement
  COLLISION_GROUP_PROJECTILE,    // Projectiles!
  COLLISION_GROUP_DOOR_BLOCKER,  // Blocks entities not permitted to get near moving doors
  COLLISION_GROUP_PASSABLE_DOOR, // Doors that the player shouldn't collide with
  COLLISION_GROUP_DISSOLVING,    // Things that are dissolving are in this group
  COLLISION_GROUP_PUSHAWAY,      // Nonsolid on client and server, pushaway in player code

  COLLISION_GROUP_NPC_ACTOR,    // Used so NPCs in scripts ignore the player.
  COLLISION_GROUP_NPC_SCRIPTED, // USed for NPCs in scripts that should not collide with each
                                // other

  LAST_SHARED_COLLISION_GROUP
};

enum tf_collision_group_t {
  TF_COLLISIONGROUP_GRENADES = LAST_SHARED_COLLISION_GROUP,
  TFCOLLISION_GROUP_OBJECT,
  TFCOLLISION_GROUP_OBJECT_SOLIDTOPLAYERMOVEMENT,
  TFCOLLISION_GROUP_COMBATOBJECT,
  TFCOLLISION_GROUP_ROCKETS, // Solid to players, but not player movement. ensures touch calls
                             // are originating from rocket
  TFCOLLISION_GROUP_RESPAWNROOMS,
  TFCOLLISION_GROUP_TANK,
  TFCOLLISION_GROUP_ROCKET_BUT_NOT_WITH_OTHER_ROCKETS,

  //
  // ADD NEW ITEMS HERE TO AVOID BREAKING DEMOS
  //
};
enum life_state_t : char {
  life_alive,
  life_dying,
  life_dead,
  life_respawnable,
  life_discardbody
};

class player_t : public base_entity_t {
public:
  netvar_value_func(uint32_t, flags, offsets.base_player.flags);
  netvar_value_func(uint32_t, tick_base, offsets.base_player.tick_base);
  netvar_value_func(life_state_t, life_state, offsets.base_player.life_state);
  netvar_value_func(int, health, offsets.base_player.health);
  netvar_value_func(float, max_speed, offsets.base_player.max_speed);
  netvar_value_func(vector3_t, eye_angles, offsets.base_player.eye_angles);
  netvar_value_func(vector3_t, view_offset, offsets.base_player.view_offset);
  netvar_value_func(base_handle_t, active_weapon_handle, offsets.base_player.active_weapon);
  netvar_value_func(usercmd_t*, current_cmd, 0x101C);
  netvar_value_func(float, next_attack_time, offsets.base_player.next_attack);
  netvar_value_func(float, stamina, offsets.base_player.stamina);
  netvar_value_func(float, fall_velocity, offsets.base_player.fall_velocity);
  netvar_value_func(float, surface_friction, offsets.base_player.surface_friction);
  netvar_value_func(vector3_t, base_velocity, offsets.base_player.base_velocity);

  int restore_data(const char* context, int slot, int type);

  typedef int(__thiscall* save_data_t)(void*, const char*, int, int);

  int save_data(const char* context, int slot, int type) {
    return ((save_data_t)offsets.functions.save_data)(this, context, slot, type);
  }

  bool hitbox_position(vector3_t& out, int hitbox, matrix_3x4_t* matrix,
                       vector3_t mins = vector3_t{0, 0, 0},
                       vector3_t maxs = vector3_t{0, 0, 0});

  vector3_t  eye_position() { return origin() + view_offset(); }
  datamap_t* get_data_map() {
    typedef datamap_t* (*datamap_func)();
    return utils::get_virtual_function<datamap_func>(this, 15)();
  }

  // netvar_value_func(vector3_t, origin, offsets.base_entity.origin);
  // netvar_value_func(vector3_t, origin, offsets.base_entity.origin);
  // netvar_value_func(vector3_t, origin, offsets.base_entity.origin);
};
