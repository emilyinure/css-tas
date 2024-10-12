#pragma once

#include "library/math.h"

class model_t; // TODO: impliment
class view_setup_t;

class render_view_t {
public:
  void get_matrices_for_view(view_setup_t& view, view_matrix_t* world_to_view,
                             view_matrix_t* view_to_projection,
                             view_matrix_t* world_to_projection,
                             view_matrix_t* world_to_pixels) {
    return utils::get_virtual_function<void(__stdcall*)(
        view_setup_t&, view_matrix_t*, view_matrix_t*, view_matrix_t*, view_matrix_t*)>(
        this, 50)(view, world_to_view, view_to_projection, world_to_projection,
                  world_to_pixels);
  }
  // virtual void draw_brush_model_ex(client_entity_t* base_entity, model_t* model, const
  // vector& origin, const q_angle& angles, draw_brush_model_mode_t mode)            = 0;
};
