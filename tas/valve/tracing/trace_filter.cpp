#include "trace_filter.h"
#include "cheat.h"
#include "valve/entities/player.h"

bool StandardFilterRules(void* pHandleEntity, int fContentsMask) {
  base_entity_t* ent = (base_entity_t*)(pHandleEntity);

  // Static prop case...
  if (!ent)
    return true;
  // const auto collide = ent->get_collideable();
  // if (!collide)
  //     return true;

  // solid_type_t solid = collide->get_solid();
  // const model_t* pModel = collide->get_collision_model();

  // if ((modelinfo->GetModelType(pModel) != mod_brush) || (solid != SOLID_BSP && solid !=
  // SOLID_VPHYSICS))
  //{
  //	if ((fContentsMask & CONTENTS_MONSTER) == 0)
  //		return false;
  // }

  // This code is used to cull out tests against see-thru entities
  // if (!(fContentsMask & CONTENTS_WINDOW) && collide->IsTransparent())
  //	return false;

  // FIXME: this is to skip BSP models that are entities that can be
  // potentially moved/deleted, similar to a monster but doors don't seem to
  // be flagged as monsters
  // FIXME: the FL_WORLDBRUSH looked promising, but it needs to be set on
  // everything that's actually a worldbrush and it currently isn't
  // if (!(fContentsMask & CONTENTS_MOVEABLE) && (collid() == MOVETYPE_PUSH))// !(touch->flags &
  // FL_WORLDBRUSH) )
  //	return false;
  //
  return true;
}

bool PassServerEntityFilter(const void* pTouch, const void* pPass) {
  if (!pPass)
    return true;

  if (pTouch == pPass)
    return false;

  const auto* pEntTouch = (base_entity_t*)(pTouch);
  const auto* pEntPass  = (base_entity_t*)(pPass);
  if (!pEntTouch || !pEntPass)
    return true;

  // don't clip against own missiles
  // if (pEntTouch->GetOwnerEntity() == pEntPass)
  //	return false;
  //
  //// don't clip against owner
  // if (pEntPass->GetOwnerEntity() == pEntTouch)
  //	return false;

  return true;
}

//-----------------------------------------------------------------------------
// The trace filter!
//-----------------------------------------------------------------------------
typedef bool(_cdecl* PassServer)(void* pTouch, void* pPass);
typedef bool(_cdecl* StandardFilter)(void* a1, int a2);
PassServer oPassServerEntityFilter = 0;
typedef bool(__stdcall* ShouldCollide)(int a1, int a2);
ShouldCollide  oShouldCollide       = 0;
StandardFilter oStandardFilterRules = 0;
bool trace_filter_simple_t::should_hit_entity(base_entity_t* pHandleEntity, int contentsMask) {
  // if (!oPassServerEntityFilter)
  //   oPassServerEntityFilter = (PassServer)offsets.functions.pass_server_entity_filter;
  // if (!oShouldCollide)
  //   oShouldCollide = (ShouldCollide)offsets.functions.should_collide;
  // if (!oStandardFilterRules)
  //   oStandardFilterRules = (StandardFilter)offsets.functions.standard_filter_rules;

  if (!oStandardFilterRules(pHandleEntity, contentsMask))
    return false;

  if (pass_ent) {
    if (!oPassServerEntityFilter(pHandleEntity, pass_ent)) {
      return false;
    }
  }

  // "55 8B EC 56 8B 75 ? 85 F6 75 ? B0 ?"; // bool __cdecl PassServerEntityFilter(int pTouch,
  // int pPass) "55 8B EC 8B 4D ? 56 8B 01 FF 50 ? 8B F0 85 F6 75 ? B0 ? 5E 5D C3"; // bool
  // __cdecl StandardFilterRules(int a1, int
  //                                                                     // a2)
  // Don't test if the game code tells us we should ignore this collision...
  base_entity_t* pEntity = (base_entity_t*)(pHandleEntity);
  if (!pEntity)
    return false;
  if (!pEntity->should_collide(collision_group, contentsMask))
    return false;
  if (pEntity && !oShouldCollide(collision_group, pEntity->collision_group()))
    return false;
  if (should_hit_extra_func && (!(should_hit_extra_func(pHandleEntity, contentsMask))))
    return false;

  return true;
}

bool trace_filter_ignore_players_t::should_hit_entity(base_entity_t* server_entity,
                                                      int            contentsMask) {
  auto networkable = server_entity->get_networkable();
  if (networkable->entity_index() != 0 &&
      networkable->entity_index() < cheat::interfaces.engine_client->get_max_clients())
    return false;

  return trace_filter_simple_t::should_hit_entity(server_entity, contentsMask);
}

bool trace_filter_ignore_teammates_t::should_hit_entity(base_entity_t* server_entity,
                                                        int            contentsMask) {
  auto networkable = server_entity->get_networkable();
  if (networkable->entity_index() != 0 &&
      networkable->entity_index() < cheat::interfaces.engine_client->get_max_clients()) {
    if (server_entity->team_number() == ignore_team) {
      return false;
    }
  }

  return trace_filter_simple_t::should_hit_entity(server_entity, contentsMask);
}
