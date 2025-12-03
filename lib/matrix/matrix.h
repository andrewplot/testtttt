#ifndef MATRIX_STUB_H
#define MATRIX_STUB_H

#include "../led_matrix/color.hh"
#include "../../Cgam/game_types.h"

// Global framebuffer that the game draws into.
// The actual storage will live in the desktop main (pc_main.cpp).
extern Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

#endif
