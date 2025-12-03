// map_render.hh - Optimized map rendering header
#ifndef MAP_RENDER_HH
#define MAP_RENDER_HH

#include "game_types.h"

/**
 * Initialize the map rendering system.
 * Generates a SINGLE combined background buffer with:
 * - Textured grass background
 * - Textured path
 * - All decorations (trees, rocks, lakes)
 * 
 * This only needs to be called ONCE at game startup.
 * 
 * @param game Pointer to game state (for path data)
 */
void map_render_init(const GameState* game);

/**
 * Copy the pre-rendered static background to the framebuffer.
 * Much faster than drawing each layer separately!
 * Call this once per frame before drawing dynamic objects.
 */
void map_render_draw_static();

// Individual decoration functions (now only used during init)
void draw_tree(int x, int y);
void draw_rock(int x, int y);
void draw_lake(int x, int y);

#endif // MAP_RENDER_HH