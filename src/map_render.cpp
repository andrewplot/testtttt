// map_render.cpp - Optimized map rendering
#include "map_render.hh"
#include "matrix.hh"
#include "game_types.h"
#include <stdlib.h>
#include <string.h>
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

// **SINGLE PRE-RENDERED BACKGROUND BUFFER**
// This contains grass + path + decorations all baked in
static Color static_background[MATRIX_HEIGHT][MATRIX_WIDTH];
static bool background_initialized = false;

// Temporary path mask (only needed during initialization)
static bool path_mask[MATRIX_HEIGHT][MATRIX_WIDTH];

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

// Bresenham line algorithm
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

// Generate path mask (which pixels are part of the 3-pixel-wide path)
static void generate_path_mask(const GameState* game) {
    // Clear mask
    memset(path_mask, 0, sizeof(path_mask));
    
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
        
        // Determine if segment is horizontal or vertical
        bool is_horizontal = (y1 == y2);
        
        // For each center point, mark a 3-pixel-wide path
        for (int p = 0; p < point_count; p++) {
            int cx = points_x[p];
            int cy = points_y[p];
            
            // Mark center pixel
            if (cx >= 0 && cx < MATRIX_WIDTH && cy >= 0 && cy < MATRIX_HEIGHT) {
                path_mask[cy][cx] = true;
            }
            
            if (is_horizontal) {
                // Add pixels above and below
                if (cy - 1 >= 0 && cx >= 0 && cx < MATRIX_WIDTH) {
                    path_mask[cy - 1][cx] = true;
                }
                if (cy + 1 < MATRIX_HEIGHT && cx >= 0 && cx < MATRIX_WIDTH) {
                    path_mask[cy + 1][cx] = true;
                }
            } else {
                // Add pixels left and right
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

// Set a pixel in the static background buffer (used during init)
static void set_static_pixel(int x, int y, Color color) {
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
        static_background[y][x] = color;
    }
}

// Draw tree directly into static background
void draw_tree(int x, int y) {
    // Tree foliage (3x3 green square)
    for (int dy = 0; dy < 3; dy++) {
        for (int dx = 0; dx < 3; dx++) {
            int px = x + dx - 1;
            int py = y + dy - 1;
            set_static_pixel(px, py, Color{TREE_GREEN_R, TREE_GREEN_G, TREE_GREEN_B});
        }
    }
    
    // Tree trunk
    int trunk_x = x;
    int trunk_y1 = y + 2;
    int trunk_y2 = y + 3;
    
    set_static_pixel(trunk_x, trunk_y1, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
    set_static_pixel(trunk_x, trunk_y2, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
    
    // Bottom trunk extension (3 pixels wide)
    for (int dx = -1; dx <= 1; dx++) {
        set_static_pixel(trunk_x + dx, trunk_y2, Color{TREE_BROWN_R, TREE_BROWN_G, TREE_BROWN_B});
    }
}

// Draw rock directly into static background
void draw_rock(int x, int y) {
    // Main rock body (2x2)
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            set_static_pixel(x + dx, y + dy, Color{ROCK_R, ROCK_G, ROCK_B});
        }
    }
    
    // Extra pixel for irregularity
    set_static_pixel(x + 2, y + 1, Color{ROCK_R, ROCK_G, ROCK_B});
}

// Draw lake directly into static background
void draw_lake(int x, int y) {
    // Row 1: 3 pixels
    for (int dx = 0; dx < 3; dx++) {
        set_static_pixel(x + dx, y - 2, Color{LAKE_R, LAKE_G, LAKE_B});
    }
    
    // Row 2: 6 pixels (main body)
    for (int dx = -1; dx < 5; dx++) {
        set_static_pixel(x + dx, y - 1, Color{LAKE_R, LAKE_G, LAKE_B});
    }
    
    // Row 3: 6 pixels
    for (int dx = -1; dx < 5; dx++) {
        set_static_pixel(x + dx, y, Color{LAKE_R, LAKE_G, LAKE_B});
    }
    
    // Row 4: 3 pixels
    for (int dx = 0; dx < 3; dx++) {
        set_static_pixel(x + dx + 1, y + 1, Color{LAKE_R, LAKE_G, LAKE_B});
    }
}

// **INITIALIZATION: Render everything ONCE into static_background**
void map_render_init(const GameState* game) {
    printf("Initializing static background...\n");
    
    // Seed random
    srand(12345);
    
    // Generate path mask
    generate_path_mask(game);
    
    // STEP 1: Draw grass background with random noise
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            int r = GRASS_R + random_range(-BG_VARIATION, BG_VARIATION);
            int g = GRASS_G + random_range(-BG_VARIATION, BG_VARIATION);
            int b = GRASS_B + random_range(-BG_VARIATION, BG_VARIATION);
            
            static_background[y][x] = Color{
                constrain_color(r),
                constrain_color(g),
                constrain_color(b)
            };
        }
    }
    
    // STEP 2: Draw path with texture on top of grass
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            if (path_mask[y][x]) {
                int base_gray = (PATH_R + PATH_G + PATH_B) / 3;
                int gray = base_gray + random_range(-PATH_VARIATION, PATH_VARIATION);
                gray = constrain_color(gray);
                
                static_background[y][x] = Color{
                    (uint8_t)gray,
                    (uint8_t)gray,
                    (uint8_t)gray
                };
            }
        }
    }
    
    // STEP 3: Draw all decorations on top
    draw_tree(5, 3);
    draw_tree(12, 2);
    draw_tree(25, 2);
    draw_tree(42, 3);
    draw_tree(60, 3);
    draw_tree(5, 28);
    draw_tree(42, 29);
    draw_tree(60, 28);
    draw_tree(24, 16);
    
    draw_rock(18, 20);
    draw_rock(35, 8);
    draw_rock(52, 1);
    draw_rock(18, 29);
    draw_rock(52, 29);
    
    draw_lake(8, 15);
    draw_lake(42, 12);
    
    background_initialized = true;
    
    printf("Static background initialized!\n");
    printf("Memory saved: ~12 KB (path_cache + background_cache eliminated)\n");
}

// **OPTIMIZED DRAW: Copy pre-rendered buffer to active framebuffer**
void map_render_draw_static() {
    if (!background_initialized) {
        printf("ERROR: map_render_init() must be called first!\n");
        return;
    }
    
    // Access the actual LED matrix framebuffer (declared in matrix.cpp)
    // We need to access frames[frame_index] directly
    extern Color frames[2][MATRIX_ROWS][MATRIX_COLS];
    extern int frame_index;
    
    // Fast memcpy to the current drawing framebuffer
    memcpy(frames[frame_index], static_background, sizeof(static_background));
}