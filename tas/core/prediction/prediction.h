#pragma once

#include "valve/prediction.h"

class prediction_t {
  float       old_current_time, old_frame_time;
  int         tick_count;
  move_data_t move_data{};
  float       time;
  bool        first_time_prediction = false;

public:
  prediction_t() = default;
  void setup();
  void set_time(float time) { this->time = time; }
  void set_first_time_prediction(bool first) { this->first_time_prediction = first; }

  bool start();
  void finish();
};
