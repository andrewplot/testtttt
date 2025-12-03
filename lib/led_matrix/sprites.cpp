#include "sprites.hh"

namespace {
    static const Color sprite_blank[3][3]  = { {GRASS_DARK, GRASS_DARK, GRASS_DARK},
                                               {GRASS_DARK, GRASS_DARK, GRASS_DARK},
                                               {GRASS_DARK, GRASS_DARK, GRASS_DARK} };

    static const Color sprite_dart[3][3]   = { {DART_RED,   DART_RED,   DART_RED},
                                               {DART_BROWN, DART_BROWN, DART_BROWN},
                                               {DART_BROWN, DART_LIGHT, DART_BROWN} };

    static const Color sprite_ninja[3][3]  = { {NINJA_RED,   NINJA_RED,   NINJA_RED},
                                               {NINJA_RED,   NINJA_RED,   NINJA_RED},
                                               {NINJA_RED,   NINJA_WHITE,  NINJA_RED} };

    static const Color sprite_bomb[3][3]   = { {BOMB_BLACK, BOMB_BLACK, BOMB_BLACK},
                                               {BOMB_BLACK, BOMB_BLACK, BOMB_BLACK},
                                               {GRASS,      GRASS,      BOMB_BROWN} };

    static const Color sprite_sniper[3][3] = { {SNIPER_DARK_GREEN, SNIPER_LIGHT_GREEN, SNIPER_DARK_GREEN},
                                               {SNIPER_BROWN,      SNIPER_BROWN,       SNIPER_BROWN},
                                               {SNIPER_BROWN,      SNIPER_LIGHT_BROWN, SNIPER_BROWN} };

    const Color sprite_tree[5][3] = {
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {GRASS,      TREE_BROWN, GRASS     },
        {TREE_BROWN, TREE_BROWN, TREE_BROWN}
    };


    const Color* const sprite_map[BLANK + 1] = {
        (const Color*) sprite_dart,   // MACHINE_GUN
        (const Color*) sprite_bomb,   // CANNON
        (const Color*) sprite_ninja,  // RADAR
        (const Color*) sprite_sniper, // SNIPER
        (const Color*) sprite_blank,  // BLANK
    };
}

const Color* get_sprite_tree() {
    return (const Color*) sprite_tree;
}

const Color* get_sprite(TowerType type) {
    return sprite_map[type];
}
