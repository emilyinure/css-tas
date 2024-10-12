#pragma once

// OVERALL Coordinate Size Limits used in COMMON.C MSG_*BitCoord() Routines (and someday the
// HUD)
#define COORD_INTEGER_BITS    14
#define COORD_FRACTIONAL_BITS 5
#define COORD_DENOMINATOR     (1 << (COORD_FRACTIONAL_BITS))
#define COORD_RESOLUTION      (1.0 / (COORD_DENOMINATOR))

// Special threshold for networking multiplayer origins
#define COORD_FRACTIONAL_BITS_MP_LOWPRECISION 3
#define COORD_DENOMINATOR_LOWPRECISION        (1 << (COORD_FRACTIONAL_BITS_MP_LOWPRECISION))
#define COORD_RESOLUTION_LOWPRECISION         (1.0 / (COORD_DENOMINATOR_LOWPRECISION))

#define NORMAL_FRACTIONAL_BITS11
#define NORMAL_DENOMINATOR ((1 << (NORMAL_FRACTIONAL_BITS)) - 1)
#define NORMAL_RESOLUTION  (1.0 / (NORMAL_DENOMINATOR))

// this is limited by the network fractional bits used for coords
// because net coords will be only be accurate to 5 bits fractional
// Standard collision test epsilon
// 1/32nd inch collision epsilon
#define DIST_EPSILON (0.03125)
