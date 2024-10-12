#pragma once

#include "library/math.h"

enum stereo_eye_t {
  STEREO_EYE_MONO  = 0,
  STEREO_EYE_LEFT  = 1,
  STEREO_EYE_RIGHT = 2,
  STEREO_EYE_MAX   = 3,
};

class view_setup_t {
public:
  int           x              = 0;
  int           unscaled_x     = 0;
  int           y              = 0;
  int           unscaled_y     = 0;
  int           width          = 0;
  int           unscaled_width = 0;
  int           height         = 0;
  stereo_eye_t  stereo_eye{};
  int           unscaled_height = 0;
  bool          ortho           = false;
  float         ortho_left      = 0;
  float         ortho_top       = 0;
  float         ortho_right     = 0;
  float         ortho_bottom    = 0;
  float         fov             = 0;
  float         fov_viewmodel   = 0;
  vector3_t     origin{};
  vector3_t     angles{};
  float         near_                             = 0;
  float         far_                              = 0;
  float         near_view_model                   = 0;
  float         far_view_model                    = 0;
  bool          render_to_subrect_of_largerScreen = false;
  float         aspect_ratio                      = 0;
  bool          off_center                        = false;
  float         off_center_top                    = 0;
  float         off_center_bottom                 = 0;
  float         off_center_left                   = 0;
  float         off_center_right                  = 0;
  bool          do_blooand_tone_mapping           = false;
  bool          cache_full_screen_state           = false;
  bool          view_to_projection_override       = false;
  view_matrix_t view_to_projection{};
};
