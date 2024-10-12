#include "player.h"
#include "cheat.h"
#include "library/math.h"
#include "valve/studio_header.h"

int player_t::restore_data(const char* context, int slot, int type) {
  typedef int(__thiscall * restore_data_t)(void*, const char*, int, int);
  return utils::get_virtual_function<restore_data_t>(this, 116)(this, context, slot, type);
}

bool player_t::hitbox_position(vector3_t& out, int hitbox_number, matrix_3x4_t* matrix,
                               vector3_t mins, vector3_t max) {
  const model_t* model = get_model();

  if (!model)
    return false;

  const studio_header_t* header = cheat::interfaces.model_info->GetStudiomodel(model);

  if (!header)
    return false;

  const mstudiobbox_t* hitbox = header->GetHitbox(hitbox_number, hitbox_set());

  if (!hitbox)
    return false;

  vector3_t center = (hitbox->bbmin * mins) + (hitbox->bbmax * max);

  out = math::vector_transform(matrix[hitbox->bone], center);

  return true;
}
