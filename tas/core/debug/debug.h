#pragma once

enum e_console_colors {
  console_color_white = 15,
  console_color_red   = 12,
  console_color_green = 10,
  console_color_cyan  = 11,
  console_color_beige = 14
};

class debug_t {
public:
  void open_console();
  void close_console();
  void set_console_color(e_console_colors color);
  void print(const char* text, e_console_colors color = e_console_colors::console_color_white);
};
