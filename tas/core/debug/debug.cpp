#include "debug.h"

#include <cstdio>
#include <Windows.h>

void debug_t::open_console() {
  AllocConsole();

  FILE* f{};
  freopen_s(&f, "CONIN$", "r", stdin);
  freopen_s(&f, "CONOUT$", "w", stdout);
  freopen_s(&f, "CONOUT$", "w", stderr);

  set_console_color(console_color_white);
}

void debug_t::close_console() {
  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
  FreeConsole();
}

void debug_t::set_console_color(e_console_colors color) {
  const auto console = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(console, color);
}

void debug_t::print(const char* text, e_console_colors color) {
  set_console_color(color);
  printf_s("%s\n", text);
}
