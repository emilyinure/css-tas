#pragma once

#include <imgui.h>

struct area_t {
  float x = 0;
  float y = 0;
  float w = 0;
  float h = 0;
};

struct screen_position_t {
  float x = 0;
  float y = 0;

  screen_position_t(float x, float y) : x(x), y(y) {}

  void set_x(float other) { x = other; }
  void set_y(float other) { y = other; }

  screen_position_t operator+(screen_position_t& other) {
    screen_position_t ret = *this;
    ret.x += other.x;
    ret.y += other.y;
    return ret;
  }
  screen_position_t operator+=(screen_position_t& other) {
    this->x += other.x;
    this->y += other.y;
    return *this;
  }
  screen_position_t operator+(float other) {
    screen_position_t ret = *this;
    ret.x += other;
    ret.y += other;
    return ret;
  }
  screen_position_t operator-(float other) {
    screen_position_t ret = *this;
    ret.x -= other;
    ret.y -= other;
    return ret;
  }
  bool operator>(screen_position_t other) { return other.x < x && other.y < y; }
  bool operator<(screen_position_t other) { return other.x > x && other.y > y; }
  bool within_area(float other_x, float other_y, float other2_x, float other2_y) {
    return x >= other_x && x <= other2_x && y >= other_y && y <= other2_y;
  }

  operator ImVec2() { return {x, y}; }
};
