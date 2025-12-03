// game_types.h
#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Use the same Color type as the LED matrix library
#include "color.hh"   // from lib/led_matrix/color.hh via PlatformIO's include paths

// Configuration constants
#define MAX_ENEMIES         50
#define MAX_TOWERS          10
#define MAX_PROJECTILES     30
#define MAX_PATH_WAYPOINTS  20

#define MATRIX_WIDTH        64
#define MATRIX_HEIGHT       32

// Global framebuffer (implemented in main.cpp)
extern Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

// ============================================================================
// Enemy system
// ============================================================================

typedef enum {
    ENEMY_SCOUT = 0,
    ENEMY_TANK,
    ENEMY_SPLITTER,
    ENEMY_GHOST
} EnemyType;

typedef struct {
    int      health;
    float    speed;
    Color    color;
    uint8_t  reward;
    uint8_t  damage;
    bool     invisible;
    bool     splits_on_death;
    uint8_t  split_count;
} EnemyStats;

// External declaration of enemy stats table (defined in game.cpp)
extern const EnemyStats ENEMY_STATS_TABLE[];

typedef struct {
    float     x;
    float     y;
    float     speed;

    int       health;
    int       max_health;

    EnemyType type;
    Color     color;

    uint8_t   path_index;
    float     path_progress;

    bool      alive;
    bool      invisible;
    bool      revealed;
} Enemy;

// ============================================================================
// Tower system (game-level)
// ============================================================================

typedef enum {
    TOWER_MACHINE_GUN = 0,
    TOWER_CANNON,
    TOWER_SNIPER,
    TOWER_RADAR,
    TOWER_BLANK  // No tower selected / invalid
} TowerType;

typedef struct {
    uint8_t   cost;
    uint8_t   damage;
    float     range;
    float     fire_rate;        // shots per second
    float     projectile_speed;
    Color     color;
    bool      can_see_invisible;
    bool      is_radar;
    uint8_t   splash_radius;    // 0 = no splash
} TowerStats;

// External declaration of tower stats table (defined in game.cpp)
extern const TowerStats TOWER_STATS_TABLE[];

typedef struct {
    float     x;
    float     y;

    TowerType type;
    Color     color;

    uint8_t   damage;
    float     range;
    float     fire_rate;
    float     projectile_speed;
    uint8_t   splash_radius;

    float     time_since_shot;
    int8_t    target_index;

    bool      can_see_invisible;
    bool      is_radar;
    float     radar_angle;
} Tower;

// ============================================================================
// Projectile system
// ============================================================================

typedef struct {
    float     x;
    float     y;
    uint8_t   target_index;
    uint8_t   damage;
    float     speed;          // 0 = instant
    Color     color;
    uint8_t   splash_radius;  // 0 = no splash
    bool      active;
} Projectile;

// ============================================================================
// Map & tower slots
// ============================================================================

typedef struct {
    int16_t x;
    int16_t y;
} PathPoint;

typedef struct {
    int16_t x;
    int16_t y;
    bool    occupied;
} TowerSlot;

// ============================================================================
// Game state
// ============================================================================

typedef struct {
    Enemy      enemies[MAX_ENEMIES];
    uint8_t    enemy_count;

    Tower      towers[MAX_TOWERS];
    uint8_t    tower_count;

    Projectile projectiles[MAX_PROJECTILES];
    uint8_t    projectile_count;

    PathPoint  path[MAX_PATH_WAYPOINTS];
    uint8_t    path_length;

    TowerSlot  tower_slots[MAX_TOWERS];
    uint8_t    tower_slot_count;

    uint16_t   money;
    uint8_t    lives;
    uint16_t   score;
    float      game_time;

    uint8_t    wave_number;
    uint8_t    total_waves;

    // UI-only: what tower type the player currently has selected
    TowerType  selected_tower;
} GameState;

// ============================================================================
// Function prototypes (implemented in game.cpp)
// ============================================================================

// Enemy functions
void enemy_init(Enemy* enemy, EnemyType type, float start_x, float start_y);
void enemy_update(Enemy* enemy, float dt, GameState* game);
void enemy_draw(const Enemy* enemy);

// Tower functions
void tower_init(Tower* tower, TowerType type, int16_t x, int16_t y);
void tower_update(Tower* tower, float dt, GameState* game);
void tower_draw(const Tower* tower);
void draw_tower_range(int16_t x, int16_t y, float range);

// Projectile functions
void projectile_init(Projectile* proj,
                     float x,
                     float y,
                     uint8_t target_idx,
                     uint8_t damage,
                     float speed,
                     Color color,
                     uint8_t splash);
bool projectile_update(Projectile* proj, float dt, GameState* game);
void projectile_draw(const Projectile* proj);

// Game functions
void game_init(GameState* game);
void game_update(GameState* game, float dt);
void game_draw(const GameState* game);
bool game_place_tower(GameState* game, TowerType type, int16_t x, int16_t y);
void game_spawn_enemy(GameState* game, EnemyType type);
void game_start_wave(GameState* game);

// Utility
float distance_squared(float x1, float y1, float x2, float y2);
float distance(float x1, float y1, float x2, float y2);
bool is_in_range(float x1, float y1, float x2, float y2, float range);

#endif // GAME_TYPES_H