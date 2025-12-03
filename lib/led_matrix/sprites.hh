#ifndef SPRITES_H
#define SPRITES_H

#include "color.hh"
#include "tower.hh"

// Get tree sprite (5x3)
const Color* get_sprite_tree();

// Get tower sprite (4x4) based on hardware tower type
const Color* get_sprite(HardwareTowerType type);

// Get tower slot sprite (4x4) - decorative platform
const Color* get_sprite_tower_slot();

#endif // SPRITES_H