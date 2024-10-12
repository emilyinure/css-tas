#pragma once

class global_vars_t {
public:
  float real_time;
  int   frame_count;
  float absolute_frame_time;
  float cur_time;
  float frame_time;
  int   max_clients;
  int   tick_count;
  float tick_interval;
  float interpolation_amount;
  int   sim_ticks_this_frame;
  int   unused0;
  int   unused1;
  bool  client;
  int   unused2;
  int   unused3;
};
