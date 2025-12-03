#include "sprites.hh"

namespace {
    // Empty slot for reference (not used in rendering)
    static const Color sprite_blank[4][4] = {
        {GRASS_DARK, GRASS_DARK, GRASS_DARK, GRASS_DARK},
        {GRASS_DARK, GRASS_DARK, GRASS_DARK, GRASS_DARK},
        {GRASS_DARK, GRASS_DARK, GRASS_DARK, GRASS_DARK},
        {GRASS_DARK, GRASS_DARK, GRASS_DARK, GRASS_DARK}
    };

    // Machine Gun - Rapid fire turret (green/brown)
    static const Color sprite_dart[4][4] = {
        {DART_RED,   DART_RED,   DART_RED,   DART_RED},      // Top: red turret top
        {DART_RED,   DART_LIGHT, DART_LIGHT, DART_RED},      // Red with highlights
        {DART_BROWN, DART_BROWN, DART_BROWN, DART_BROWN},    // Brown base
        {DART_BROWN, DART_LIGHT, DART_LIGHT, DART_BROWN}     // Base with highlights
    };

    // Cannon - Heavy artillery (black/gray)
    static const Color sprite_bomb[4][4] = {
        {GRASS,      BOMB_BLACK, BOMB_BLACK, GRASS},         // Top: barrel
        {BOMB_BLACK, BOMB_BLACK, BOMB_BLACK, BOMB_BLACK},    // Black turret body
        {BOMB_BROWN, BOMB_BROWN, BOMB_BROWN, BOMB_BROWN},    // Brown base
        {BOMB_BROWN, Color(200,140,80), Color(200,140,80), BOMB_BROWN}  // Base highlights
    };

    // Sniper - Long range (green camouflage)
    static const Color sprite_sniper[4][4] = {
        {GRASS,              SNIPER_DARK_GREEN,  SNIPER_DARK_GREEN,  GRASS},               // Scope
        {SNIPER_LIGHT_GREEN, SNIPER_DARK_GREEN,  SNIPER_DARK_GREEN,  SNIPER_LIGHT_GREEN}, // Camo pattern
        {SNIPER_BROWN,       SNIPER_LIGHT_BROWN, SNIPER_LIGHT_BROWN, SNIPER_BROWN},       // Brown base
        {SNIPER_BROWN,       SNIPER_BROWN,       SNIPER_BROWN,       SNIPER_BROWN}        // Solid base
    };

    // Radar - Detection tower (red/white with rotating dish)
    static const Color sprite_ninja[4][4] = {
        {NINJA_WHITE, NINJA_RED,   NINJA_RED,   NINJA_WHITE},    // Radar dish
        {NINJA_RED,   NINJA_WHITE, NINJA_WHITE, NINJA_RED},      // Dish pattern
        {NINJA_RED,   NINJA_RED,   NINJA_RED,   NINJA_RED},      // Red tower body
        {NINJA_RED,   NINJA_WHITE, NINJA_WHITE, NINJA_RED}       // Base with detail
    };

    // Tree sprite (5x3) - unchanged
    const Color sprite_tree[5][3] = {
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {TREE_GREEN, TREE_GREEN, TREE_GREEN},
        {GRASS,      TREE_BROWN, GRASS     },
        {TREE_BROWN, TREE_BROWN, TREE_BROWN}
    };

    // Tower slot sprite (4x4) - decorative platform
    const Color sprite_tower_slot[4][4] = {
        {Color(180,150,0), Color(128,107,0), Color(128,107,0), Color(180,150,0)},  // Gold border
        {Color(128,107,0), Color(90,75,0),   Color(90,75,0),   Color(128,107,0)},  // Darker center
        {Color(128,107,0), Color(90,75,0),   Color(90,75,0),   Color(128,107,0)},  // Darker center
        {Color(180,150,0), Color(128,107,0), Color(128,107,0), Color(180,150,0)}   // Gold border
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

const Color* get_sprite(HardwareTowerType type) {
    return sprite_map[type];
}

const Color* get_sprite_tower_slot() {
    return (const Color*) sprite_tower_slot;
}