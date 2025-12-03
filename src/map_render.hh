// map_render.hh - Map rendering header
#ifndef MAP_RENDER_HH
#define MAP_RENDER_HH

#include "game_types.h"

/**
 * Initialize the map rendering system.
 * Generates cached background and path textures with random noise.
 * Call this once at game startup.
 * 
 * @param game Pointer to game state (for path data)
 */
void map_render_init(const GameState* game);

/**
 * Draw the complete map (background + path) to the framebuffer.
 * Uses cached textures for consistent appearance.
 * 
 * @param game Pointer to game state
 */
void map_render_draw(const GameState* game);

/**
 * Draw decorative elements (trees, rocks, etc.)
 * Call after map_render_draw() but before drawing game objects.
 */
void map_render_decorations();

/**
 * Draw a single tree at position (x, y)
 * 
 * @param x X coordinate (center of tree)
 * @param y Y coordinate (top of tree foliage)
 */
void draw_tree(int x, int y);

/**
 * Draw a rock at position (x, y)
 * 
 * @param x X coordinate
 * @param y Y coordinate
 */
void draw_rock(int x, int y);

/**
 * Draw a lake at position (x, y)
 * 
 * @param x X coordinate (center)
 * @param y Y coordinate (center)
 */
void draw_lake(int x, int y);

#endif // MAP_RENDER_HH