#pragma once

#include "library/math.h"

#pragma pack(push, 1)
class view_vectors_t {
public:
  vector3_t view;

  vector3_t hull_min;
  vector3_t hull_max;

  vector3_t duck_hull_min;
  vector3_t duck_hull_max;
  vector3_t duck_view;

  vector3_t obs_hull_min;
  vector3_t obs_hull_max;

  vector3_t dead_view_height;
};
#pragma pack(pop)

class ccsgamerules_t {
public:
  virtual void            pad_00()                    = 0;
  virtual void            pad_01()                    = 0;
  virtual void            pad_02()                    = 0;
  virtual void            pad_03()                    = 0;
  virtual void            pad_04()                    = 0;
  virtual void            pad_05()                    = 0;
  virtual void            pad_06()                    = 0;
  virtual void            pad_07()                    = 0;
  virtual void            pad_08()                    = 0;
  virtual void            pad_09()                    = 0;
  virtual void            pad_10()                    = 0;
  virtual void            pad_11()                    = 0;
  virtual void            pad_12()                    = 0;
  virtual void            pad_13()                    = 0;
  virtual void            pad_14()                    = 0;
  virtual void            pad_15()                    = 0;
  virtual void            pad_16()                    = 0;
  virtual void            pad_17()                    = 0;
  virtual void            pad_18()                    = 0;
  virtual void            pad_19()                    = 0;
  virtual void            pad_20()                    = 0;
  virtual void            pad_21()                    = 0;
  virtual void            pad_22()                    = 0;
  virtual void            pad_23()                    = 0;
  virtual void            pad_24()                    = 0;
  virtual void            pad_25()                    = 0;
  virtual void            pad_26()                    = 0;
  virtual void            pad_27()                    = 0;
  virtual void            pad_28()                    = 0;
  virtual void            pad_29()                    = 0;
  virtual void            pad_30()                    = 0;
  virtual view_vectors_t* get_view_vectors() noexcept = 0;
};

class i_prediction_t {
public:
  virtual ~i_prediction_t(){};
  virtual void init()                       = 0;
  virtual void shutdown()                   = 0;
  virtual void update(int start_frame, bool valid_frame, int incoming_acknowledged,
                      int outgoing_command) = 0;

  virtual void pre_entity_packet_received(int commands_acknowledged,
                                          int current_world_update_packet) = 0;
  virtual void post_entity_packet_received()                               = 0;
  virtual void post_network_data_received(int commands_acknowledged)       = 0;
  virtual void on_received_uncompressed_packet()                           = 0;
  virtual void get_view_origin(vector3_t& org)                             = 0;
  virtual void set_view_origin(vector3_t& org)                             = 0;
  virtual void get_view_angles(vector3_t& ang)                             = 0;
  virtual void set_view_angles(vector3_t& ang)                             = 0;
  virtual void get_local_view_angles(vector3_t& ang)                       = 0;
  virtual void set_local_view_angles(vector3_t& ang)                       = 0;
};

class player_t;
class usercmd_t;
class move_helper_t;

class move_data_t {

public:
  // bool is_first_run_of_functions : 1;
  // bool is_game_code_moved_player : 1;

  // void* player_handle; // edict index on server, client entity handle on client

  // int       impulse_command; // Impulse command issued.
  // vector3_t view_angles;     // Command view angles (local space)
  // vector3_t abs_view_angles; // Command view angles (world space)
  // int       buttons;         // Attack buttons.
  // int       old_buttons;     // From host_client->oldbuttons;
  // float     forward_move;
  // float     old_forward_move;
  // float     side_move;
  // float     up_move;

  // float max_speed;
  // float client_max_speed;

  // // Variables from the player edict (sv_player) or entvars on the client.
  // // These are copied in here before calling and copied out after calling.
  // vector3_t velocity;
  // vector3_t angles;
  // vector3_t old_angles;

  // // Output only
  // float     out_step_height; // how much you climbed this move
  // vector3_t out_wish_vel;    // This is where you tried
  // vector3_t out_jump_vel;    // This is your jump velocity

  // // Movement constraints
  // vector3_t constraint_center;
  // float     constraint_radius;
  // float     constraint_width;
  // // float     constraint_speed_factor;

  // // private:
  // vector3_t abs_origin; // edict::origin
  char      pad_0[0xc];
  vector3_t view_angles; // 0xc
  char      pad_18[0xc];
  int       buttons; // 0x24
  char      pad_28[0x4];
  float     forward_move; // 0x2c
  float     side_move;    // 0x30
  float     up_move;      // 0x34
  char      pad_38[0x8];
  vector3_t velocity;   // 0x40
  vector3_t angles;     // 0x4c
  vector3_t old_angles; // 0x58
  char      pad_64[0x34];
  vector3_t abs_origin; // 0x98
};

class move_helper_t {
public:
  virtual void set_host(player_t*) = 0;
};

class prediction_manager_t : i_prediction_t {
public:
  virtual ~prediction_manager_t();
  virtual void init();
  virtual void shutdown();
  virtual void update(int, bool, int, int);

  virtual void on_received_uncompressed_packet()                               = 0;
  virtual void pre_entity_packet_received(int, int)                            = 0;
  virtual void post_entity_packet_received()                                   = 0;
  virtual void post_network_data_received(int)                                 = 0;
  virtual bool in_prediction()                                                 = 0;
  virtual bool is_first_time_predicted()                                       = 0;
  virtual int  get_incoming_packet_number()                                    = 0;
  virtual void get_view_origin(vector3_t&)                                     = 0;
  virtual void set_view_origin(vector3_t&)                                     = 0;
  virtual void get_view_angles(vector3_t&)                                     = 0;
  virtual void set_view_angles(vector3_t&)                                     = 0;
  virtual void get_local_view_angles(vector3_t&)                               = 0;
  virtual void set_local_view_angles(vector3_t&)                               = 0;
  virtual void run_command(player_t*, usercmd_t*, move_helper_t*)              = 0;
  virtual void setup_move(player_t*, usercmd_t*, move_helper_t*, move_data_t*) = 0;
  virtual void finish_move(player_t*, usercmd_t*, move_data_t*)                = 0;
  virtual void set_ideal_pitch(player_t*, const vector3_t&, const vector3_t&,
                               const vector3_t&)                               = 0;
  virtual void _update(bool, bool, int, int)                                   = 0;

  uint32_t last_ground;
  bool     is_in_prediction;
  bool     first_time_predicted;
  bool     old_cl_predict_value;
  bool     engine_paused;
  int      previous_start_frame;
  int      commands_predicted;
  int      server_commands_acknowledged;
  int      previous_ack_had_errors;
  int      incoming_packet_number;
  float    ideal_pitch;
};

class game_movement_t {
public:
  virtual ~game_movement_t(void) {}

  // Process the current movement command
  virtual void proccess_movement(player_t* pPlayer, move_data_t* pMove) = 0;
  virtual void start_track_prediction_errors(player_t* pPlayer)         = 0;
  virtual void finish_tack_prediction_errors(player_t* pPlayer)         = 0;
  virtual void diff_print(char const* fmt, ...)                         = 0;

  // Allows other parts of the engine to find out the normal and ducked player bbox sizes
  virtual vector3_t get_player_mins(bool ducked) const        = 0;
  virtual vector3_t get_player_maxs(bool ducked) const        = 0;
  virtual vector3_t get_player_view_offset(bool ducked) const = 0;
};
