#include "prediction.h"
#include "cheat.h"
#include "valve/entities/player.h"
#include "valve/usercmd.h"

void prediction_t::setup() {}

bool prediction_t::start() {
  if (!cheat::local_player || !cheat::interfaces.move_helper)
    return false;

  cheat::interfaces.move_helper->set_host(cheat::local_player);
  memset(&move_data, 0, sizeof(move_data));

  cheat::local_player->current_cmd() = cheat::cmd;

  old_frame_time   = cheat::interfaces.global_vars->frame_time;
  old_current_time = cheat::interfaces.global_vars->cur_time;

  const bool old_is_first_prediction = cheat::interfaces.prediction->is_first_time_predicted();
  const bool old_in_prediction       = cheat::interfaces.prediction->in_prediction();

  cheat::interfaces.global_vars->cur_time   = time;
  cheat::interfaces.global_vars->frame_time = cheat::interfaces.global_vars->tick_interval;

  cheat::interfaces.prediction->is_in_prediction     = true;
  cheat::interfaces.prediction->first_time_predicted = first_time_prediction;

  cheat::interfaces.prediction->set_local_view_angles(cheat::cmd->view_angles);

  cheat::interfaces.game_movement->start_track_prediction_errors(cheat::local_player);

  if (first_time_prediction)
    cheat::interfaces.prediction->setup_move(cheat::local_player, cheat::cmd,
                                             cheat::interfaces.move_helper, &move_data);
  cheat::interfaces.game_movement->proccess_movement(cheat::local_player, &move_data);

  cheat::interfaces.prediction->finish_move(cheat::local_player, cheat::cmd, &move_data);

  cheat::interfaces.game_movement->finish_tack_prediction_errors(cheat::local_player);

  cheat::interfaces.prediction->is_in_prediction     = old_in_prediction;
  cheat::interfaces.prediction->first_time_predicted = old_is_first_prediction;

  cheat::interfaces.global_vars->cur_time   = old_current_time;
  cheat::interfaces.global_vars->frame_time = old_frame_time;

  return true;
}

void prediction_t::finish() {
  if (!cheat::interfaces.move_helper)
    return;

  cheat::interfaces.move_helper->set_host(nullptr);
}
