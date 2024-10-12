#pragma once

#include <memory>
#include <vector>

class form_t;

namespace menu {
  inline bool                                 open{};
  inline bool                                 allow_passthrough{};
  inline auto                                 nudge_step{0.25f};
  inline std::vector<std::shared_ptr<form_t>> forms{};
  void                                        initialize();
  void                                        on_present();
} // namespace menu
