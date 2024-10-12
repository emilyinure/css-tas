#pragma once

#include "library/math.h"

// todo: add these.
class i_material;
struct surf_info;
class model_t;
class net_channel_info_t;

typedef struct player_info_s {
  char          m_name[32];
  int           m_user_id;
  char          m_guid[33];
  uint32_t      m_friends_id;
  char          m_friends_name[32];
  bool          m_fake_player;
  bool          m_is_hltv;
  bool          m_is_replay;
  uint32_t      m_custom_files[4];
  unsigned char m_files_downloaded;
} player_info_t;

class engine_client_t {
public:
  virtual int            get_intersecting_surfaces()                                       = 0;
  virtual void           get_light_for_point()                                             = 0;
  virtual i_material*    trace_line_material_and_lighting()                                = 0;
  virtual const char*    parse_file(const char* data, char* token, int maxlen)             = 0;
  virtual bool           copy_local_file(const char* source, const char* destination)      = 0;
  virtual void           get_screen_size(int& width, int& height)                          = 0;
  virtual void           server_cmd(const char* cmd_string, bool reliable = true)          = 0;
  virtual void           client_cmd(const char* cmd_string)                                = 0;
  virtual bool           get_player_info(int ent_num, player_info_t* info)                 = 0;
  virtual int            get_player_for_user_id(int user_id)                               = 0;
  virtual void*          text_message_get(const char* name)                                = 0;
  virtual bool           con_is_visible(void)                                              = 0;
  virtual int            get_local_player(void)                                            = 0;
  virtual const model_t* load_model(const char* name, bool prop = false)                   = 0;
  virtual float          time(void)                                                        = 0;
  virtual float          get_last_time_stamp(void)                                         = 0;
  virtual void*          get_sentence()                                                    = 0;
  virtual float          get_sentence_length()                                             = 0;
  virtual bool           is_streaming() const                                              = 0;
  virtual void           get_view_angles(vector3_t& va)                                    = 0;
  virtual void           set_view_angles(vector3_t& va)                                    = 0;
  virtual int            get_max_clients(void)                                             = 0;
  virtual const char*    key_lookup_binding(const char* binding)                           = 0;
  virtual const char*    key_binding_for_key()                                             = 0;
  virtual void           start_key_trap_mode(void)                                         = 0;
  virtual bool           check_done_key_trapping()                                         = 0;
  virtual bool           is_in_game(void)                                                  = 0;
  virtual bool           is_connected(void)                                                = 0;
  virtual bool           is_drawing_loading_image(void)                                    = 0;
  virtual void           con_n_printf(int pos, const char* fmt, ...)                       = 0;
  virtual void        con_nx_printf(const struct con_nprint_s* info, const char* fmt, ...) = 0;
  virtual int         is_box_visible()                                                     = 0;
  virtual int         is_box_in_view_cluster()                                             = 0;
  virtual bool        cull_box()                                                           = 0;
  virtual void        sound_extra_update(void)                                             = 0;
  virtual const char* get_game_directory(void)                                             = 0;
  virtual const view_matrix_t&    world_to_screen_matrix()                                 = 0;
  virtual const view_matrix_t&    world_to_view_matrix()                                   = 0;
  virtual int                     game_lump_version(int lump_id) const                     = 0;
  virtual int                     game_lump_size(int lump_id) const                        = 0;
  virtual bool                    load_game_lump(int lump_id, void* buffer, int size)      = 0;
  virtual int                     level_leaf_count() const                                 = 0;
  virtual void*                   get_bsp_tree_query()                                     = 0;
  virtual void                    linear_to_gamma(float* linear, float* gamma)             = 0;
  virtual float                   light_style_value(int style)                             = 0;
  virtual void                    compute_dynamic_lighting()                               = 0;
  virtual void                    get_ambient_light_color()                                = 0;
  virtual int                     get_dx_support_level()                                   = 0;
  virtual bool                    supports_hdr()                                           = 0;
  virtual void                    mat_stub()                                               = 0;
  virtual void                    get_chapter_name(char* pch_buff, int i_max_length)       = 0;
  virtual char const*             get_level_name(void)                                     = 0;
  virtual int                     get_level_version(void)                                  = 0;
  virtual struct i_voice_tweak_s* get_voice_tweak_api(void)                                = 0;
  virtual void                    engine_stats_begin_frame(void)                           = 0;
  virtual void                    engine_stats_end_frame(void)                             = 0;
  virtual void                    fire_events()                                            = 0;
  virtual int                     get_leaves_area(int* leaves, int num_leaves)             = 0;
  virtual bool                    does_box_touch_area_frustum()                            = 0;
  virtual void                    set_audio_state()                                        = 0;
  virtual int sentence_group_pick(int group_index, char* name, int name_buf_len)           = 0;
  virtual int sentence_group_pick_sequential(int group_index, char* name, int name_buf_len,
                                             int sentence_index, int reset)                = 0;
  virtual int sentence_index_from_name(const char* sentence_name)                          = 0;
  virtual const char*  sentence_name_from_index(int sentence_index)                        = 0;
  virtual int          sentence_group_index_from_name(const char* group_name)              = 0;
  virtual const char*  sentence_group_name_from_index(int group_index)                     = 0;
  virtual float        sentence_length(int sentence_index)                                 = 0;
  virtual void         compute_lighting()                                                  = 0;
  virtual void         activate_occluder(int occluder_index, bool active)                  = 0;
  virtual bool         is_occluded()                                                       = 0;
  virtual void*        save_alloc_memory(size_t num, size_t size)                          = 0;
  virtual void         save_free_memory(void* save_mem)                                    = 0;
  virtual void*        get_net_channel_info(void)                                          = 0;
  virtual void         debug_draw_phys_collide()                                           = 0;
  virtual void         check_point(const char* name)                                       = 0;
  virtual void         draw_portals()                                                      = 0;
  virtual bool         is_playing_demo(void)                                               = 0;
  virtual bool         is_recording_demo(void)                                             = 0;
  virtual bool         is_playing_time_demo(void)                                          = 0;
  virtual int          get_demo_recording_tick(void)                                       = 0;
  virtual int          get_demo_playback_tick(void)                                        = 0;
  virtual int          get_demo_playback_start_tick(void)                                  = 0;
  virtual float        get_demo_playback_time_scale(void)                                  = 0;
  virtual int          get_demo_playback_total_ticks(void)                                 = 0;
  virtual bool         is_paused(void)                                                     = 0;
  virtual bool         is_taking_screenshot(void)                                          = 0;
  virtual bool         is_hltv(void)                                                       = 0;
  virtual bool         is_level_main_menu_background(void)                                 = 0;
  virtual void         get_main_menu_background_name(char* dest, int destlen)              = 0;
  virtual void         get_video_modes()                                                   = 0;
  virtual void         set_occlusion_parameters()                                          = 0;
  virtual void         get_ui_language(char* dest, int destlen)                            = 0;
  virtual void         is_skybox_visible_from_point()                                      = 0;
  virtual const char*  get_map_entities_string()                                           = 0;
  virtual bool         is_in_edit_mode(void)                                               = 0;
  virtual float        get_screen_aspect_ratio()                                           = 0;
  virtual bool         removed_steam_refresh_login(const char* password, bool is_secure)   = 0;
  virtual bool         removed_steam_process_call(bool& finished)                          = 0;
  virtual unsigned int get_engine_build_number()                                           = 0;
  virtual const char*  get_product_version_string()                                        = 0;
  virtual void         grab_pre_color_corrected_frame(int x, int y, int width, int height) = 0;
  virtual bool         is_hammer_running() const                                           = 0;
  virtual void         execute_client_cmd(const char* cmd_string)                          = 0;
  virtual bool         map_has_hdr_lighting(void)                                          = 0;
  virtual int          get_app_id()                                                        = 0;
  virtual void         get_light_for_point_fast()                                          = 0;
  virtual void         client_cmd_unrestricted(const char* cmd_string)                     = 0;
  // net_channel_info_t* GetNetChannelInfo(void)
  // {
  //     typedef net_channel_info_t*(__thiscall * OriginalFn)(PVOID);
  //     return g_utils.get_virtual_function<OriginalFn>(this, 72)(this);
  // }
};
