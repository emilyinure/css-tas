#pragma once

#include <algorithm>
class color {
public:
  int r, g, b, a;

  color(const int r, const int g, const int b, const int a = 255) : r(r), g(g), b(b), a(a) {}

  color operator-(const int val) const {
    color return_val = {255, 255, 255};
    return_val.r     = std::clamp(this->r - val, 0, 255);
    return_val.g     = std::clamp(this->g - val, 0, 255);
    return_val.b     = std::clamp(this->b - val, 0, 255);

    return return_val;
  }
};
