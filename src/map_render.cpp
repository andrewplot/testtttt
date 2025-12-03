// map_render.cpp - Map rendering with path segments and random noise
#include "map_render.hh"
#include "matrix.hh"
#include "game_types.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Color definitions (darker to account for gamma correction)
#define GRASS_R 0
#define GRASS_G 60
#define GRASS_B 0

#define PATH_R 45
#define PATH_G 45
#define PATH_B 45

#define TREE_GREEN_R 0
#define TREE_GREEN_G 80
#define TREE_GREEN_B 0

#define TREE_BROWN_R 100
#define TREE_BROWN_G 50
#define TREE_BROWN_B 0

#define ROCK_R 30
#define ROCK_G 30
#define ROCK_B 30

#define LAKE_R 0
#define LAKE_G 0
#define LAKE_B 80

// Random noise variation amounts
#define BG_VARIATION 8
#define PATH_VARIATION 3

// Cached background and path textures
static Color background_cache[MATRIX_HEIGHT][MATRIX_WIDTH];
static Color path_cache[MATRIX_HEIGHT][MATRIX_WIDTH];
static bool path_mask[MATRIX_HEIGHT][MATRIX_WIDTH];  // Track which pixels are path
static bool cache_initialized = false;

// Helper: constrain value to [0, 255]
static inline uint8_t constrain_color(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

// Helper: simple random in range [min, max] inclusive
static inline int random_range(int min, int max) {
    return min + (rand() % (max - min + 1));
}

// Bresenham line algorithm - returns all points along line
static void get_line_points(int x0, int y0, int x1, int y1, 
                           int* points_x, int* points_y, int* count) {
    *count = 0;
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (true) {
        points_x[*count] = x;
        points_y[*count] = y;
        (*count)++;
        
        if (x == x1 && y == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

// Mark all pixels that are part of the 3-pixel-wide path
static void generate_path_mask(const GameState* game) {
    // Clear mask
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            path_mask[y][x] = false;
        }
    }
    
    // For each path segment
    for (int i = 0; i < game->path_length - 1; i++) {
        int x1 = game->path[i].x;
        int y1 = game->path[i].y;
        int x2 = game->path[i + 1].x;
        int y2 = game->path[i + 1].y;
        
        // Get center line points
        int points_x[256];
        int points_y[256];
        int point_count;
        get_line_points(x1, y1, x2, y2, points_x, points_y, &point_count);
        
        // Determine if segment is horizontal or vertical (check FIRST point pair)
        bool is_horizontal = (y1 == y2);
        
        // For each center point, mark a 3-pixel-wide path
        for (int p = 0; p < point_count; p++) {
            int cx = points_x[p];
            int cy = points_y[p];
            
            // Mark center pixel (where enemy moves)
            if (cx >= 0 && cx < MATRIX_WIDTH && cy >= 0 && cy < MATRIX_HEIGHT) {
                path_mask[cy][cx] = true;
            }
            
            if (is_horizontal) {
                // Horizontal segment: add pixels above (cy-1) and below (cy+1)
                if (cy - 1 >= 0 && cx >= 0 && cx < MATRIX_WIDTH) {
                    path_mask[cy - 1][cx] = true;
                }
                if (cy + 1 < MATRIX_HEIGHT && cx >= 0 && cx < MATRIX_WIDTH) {
                    path_mask[cy + 1][cx] = true;
                }
            } else {
                // Vertical segment: add pixels left (cx-1) and right (cx+1)
                if (cx - 1 >= 0 && cy >= 0 && cy < MATRIX_HEIGHT) {
                    path_mask[cy][cx - 1] = true;
                }
                if (cx + 1 < MATRIX_WIDTH && cy >= 0 && cy < MATRIX_HEIGHT) {
                    path_mask[cy][cx + 1] = true;
                }
            }
        }
    }
}


// Generate textured background with random noise (once)
static void generate_background_texture() {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            int r = GRASS_R + random_range(-BG_VARIATION, BG_VARIATION);
            int g = GRASS_G + random_range(-BG_VARIATION, BG_VARIATION);
            int b = GRASS_B + random_range(-BG_VARIATION, BG_VARIATION);
            
            background_cache[y][x] = Color{
                constrain_color(r),
                constrain_color(g),
                constrain_color(b)
            };
        }
    }
}

// Generate textured path with random noise (once)
static void generate_path_texture() {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            if (path_mask[y][x]) {
                // Start with base gray value (average of PATH_R, PATH_G, PATH_B)
                int base_gray = (PATH_R + PATH_G + PATH_B) / 3;
                
                // Add subtle random variation to the gray value
                int gray = base_gray + random_range(-PATH_VARIATION, PATH_VARIATION);
                gray = constrain_color(gray);
                
                // Use the same value for R, G, and B to ensure pure gray
                path_cache[y][x] = Color{
                    (uint8_t)gray,
                    (uint8_t)gray,
                    (uint8_t)gray
                };
            } else {
                path_cache[y][x] = Color{0, 0, 0};  // Not used
            }
        }
    }
}


// Initialize the map rendering system
void map_render_init(const GameState* game) {
    // Seed random number generator with something
    // (In real code, you might use a hardware RNG or timer)
    srand(12345);
    
    // Generate path mask first
    generate_path_mask(game);
    
    // Generate textures
    generate_background_texture();
    generate_path_texture();
    
    cache_initialized = true;
    
    printf("Map rendering initialized\n");
}

// Draw the complete map (background + path)
void map_render_draw(const GameState* game) {
    if (!cache_initialized) {
        map_render_init(game);
    }
    
    // Draw cached textures to framebuffer
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            if (path_mask[y][x]) {
                // Draw path texture
                set_pixel(x, y, path_cache[y][x]);
            } else {
                // Draw background texture
                set_pixel(x, y, background_cache[y][x]);
            }
        }
    }
}

// Draw decorations (trees, etc.) - called after map_render_draw()
void map_render_decorations() {
    // Trees scattered around the map (away from path and tower slots)
    draw_tree(5, 3);
    draw_tree(12, 2);
    draw_tree(25, 2);
    draw_tree(42, 3);
    draw_tree(60, 3);
    draw_tree(5, 28);
    draw_tree(42, 29);
    draw_tree(60, 28);
    draw_tree(24, 16);

    
    // Rocks
    draw_rock(18, 20);
    draw_rock(35, 8);
    draw_rock(52, 1);
    draw_rock(18, 29);
    draw_rock(52, 29);
    
    // Lakes
    draw_lake(8, 15);
    draw_lake(42, 12);
}

// Draw a single tree (5x3 sprite)
void draw_tree(int x, int y) {
    // Tree foliage (3x3 green square)
    for (int dy = 0; dy < 3; dy++) {
        for (int dx = 0; dx < 3; dx++) {
            int px = x + dx - 1;  // Center the tree
            int py = y + dy - 1;
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                set_pixel(px, py, Color{TREE_GREEN_R, TREE_GREEN_G, TREE_GREEN_B});
            }
        }
    }
    
    // Tree trunk (brown, 2 pixels tall)
    int trunk_x = x;
    int trunk_y1 = y + 2;
    int trunk_y2 = y + 3;
    
    if (trunk_x >= 0 && trunk_x < MATRIX_WIDTH) {
        if (trunk_y1 >= 0 && trunk_y1 < MATRIX_HEIGHT) {
            set_pixel(trunk_x, trunk_y1, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
        }
        if (trunk_y2 >= 0 && trunk_y2 < MATRIX_HEIGHT) {
            set_pixel(trunk_x, trunk_y2, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
        }
    }
    
    // Bottom trunk extension (3 pixels wide)
    for (int dx = -1; dx <= 1; dx++) {
        int px = trunk_x + dx;
        int py = trunk_y2;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            set_pixel(px, py, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
        }
    }
}

// Draw a rock (2x2 sprite with extra pixel)
void draw_rock(int x, int y) {
    // Main rock body (2x2)
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                set_pixel(px, py, Color{ROCK_R, ROCK_G, ROCK_B});
            }
        }
    }
    
    // Extra pixel to make it irregular
    if (x + 2 >= 0 && x + 2 < MATRIX_WIDTH && y + 1 >= 0 && y + 1 < MATRIX_HEIGHT) {
        set_pixel(x + 2, y + 1, Color{ROCK_R, ROCK_G, ROCK_B});
    }
}

// Draw a small lake (irregular shape)
void draw_lake(int x, int y) {
    // Lake shape (roughly 6x3)
    // Row 1: 3 pixels
    for (int dx = 0; dx < 3; dx++) {
        int px = x + dx;
        int py = y - 2;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            set_pixel(px, py, Color{LAKE_R, LAKE_G, LAKE_B});
        }
    }
    
    // Row 2: 6 pixels (main body)
    for (int dx = -1; dx < 5; dx++) {
        int px = x + dx;
        int py = y - 1;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            set_pixel(px, py, Color{LAKE_R, LAKE_G, LAKE_B});
        }
    }
    
    // Row 3: 6 pixels
    for (int dx = -1; dx < 5; dx++) {
        int px = x + dx;
        int py = y;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            set_pixel(px, py, Color{LAKE_R, LAKE_G, LAKE_B});
        }
    }
    
    // Row 4: 3 pixels
    for (int dx = 0; dx < 3; dx++) {
        int px = x + dx + 1;
        int py = y + 1;
        if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
            set_pixel(px, py, Color{LAKE_R, LAKE_G, LAKE_B});
        }
    }
}