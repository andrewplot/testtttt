#ifndef SPRITES_H
#define SPRITES_H

#include "color.hh"
#include "tower.hh"

const Color* get_sprite_tree();

const Color* get_sprite(TowerType type);

#endif // SPRITES_H