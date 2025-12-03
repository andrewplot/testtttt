// game.cpp - Core game implementation with FIXED PROJECTILES
#include "game_types.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "matrix.hh"
#include "sprites.hh"

// Helper function to convert game TowerType to hardware HardwareTowerType
static HardwareTowerType game_to_hardware_tower(TowerType game_type) {
    switch (game_type) {
        case TOWER_MACHINE_GUN: return MACHINE_GUN;
        case TOWER_CANNON:      return CANNON;
        case TOWER_SNIPER:      return SNIPER;
        case TOWER_RADAR:       return RADAR;
        case TOWER_BLANK:
        default:                return BLANK;
    }
}

void tower_draw(const Tower* tower) {
    // Get the appropriate sprite based on tower type
    HardwareTowerType hw_type = game_to_hardware_tower(tower->type);
    const Color* sprite = get_sprite(hw_type);
    
    int base_x = (int)tower->x - 2;  // Center the 4x4 sprite
    int base_y = (int)tower->y - 2;
    
    // Draw the 4x4 tower sprite
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            int px = base_x + dx;
            int py = base_y + dy;
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                Color pixel_color = sprite[dy * 4 + dx];
                set_pixel(px, py, pixel_color);
            }
        }
    }
    
    // Draw radar sweep line if it's a radar tower
    if (tower->is_radar) {
        int cx = (int)tower->x;
        int cy = (int)tower->y;
        int sweep_length = (int)(tower->range) - 1;
        int end_x = cx + (int)(cosf(tower->radar_angle) * sweep_length);
        int end_y = cy + (int)(sinf(tower->radar_angle) * sweep_length);
        
        // Draw sweep line in cyan
        matrix_draw_line(cx, cy, end_x, end_y, Color(0, 120, 120));
    }
}

void game_draw(const GameState* game) {
    // Draw tower slots using sprite
    const Color* slot_sprite = get_sprite_tower_slot();
    
    for (int i = 0; i < game->tower_slot_count; i++) {
        int x = game->tower_slots[i].x;
        int y = game->tower_slots[i].y;
        
        // Draw 4x4 tower slot sprite centered on slot position
        for (int dy = 0; dy < 4; dy++) {
            for (int dx = 0; dx < 4; dx++) {
                int px = x + dx - 2;  // Center the sprite
                int py = y + dy - 2;
                if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                    // If slot is occupied, darken the sprite
                    Color slot_color = slot_sprite[dy * 4 + dx];
                    if (game->tower_slots[i].occupied) {
                        slot_color.r /= 3;
                        slot_color.g /= 3;
                        slot_color.b /= 3;
                    }
                    set_pixel(px, py, slot_color);
                }
            }
        }
    }

    // Draw towers
    for (int i = 0; i < game->tower_count; i++) {
        tower_draw(&game->towers[i]);
    }

    // Draw enemies
    for (int i = 0; i < game->enemy_count; i++) {
        enemy_draw(&game->enemies[i]);
    }

    // Draw projectiles
    for (int i = 0; i < game->projectile_count; i++) {
        projectile_draw(&game->projectiles[i]);
    }
}

// Helper function to draw a line (needed for radar sweep)
void matrix_draw_line(int x0, int y0, int x1, int y1, Color color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        if (x0 >= 0 && x0 < MATRIX_WIDTH && y0 >= 0 && y0 < MATRIX_HEIGHT) {
            set_pixel(x0, y0, color);
        }
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// ============================================================================
// ENEMY STATS TABLE
// ============================================================================

const EnemyStats ENEMY_STATS_TABLE[] = {
    // ENEMY_SCOUT
    {
        .health = 3,
        .speed = 4.0f,
        .color = {200, 20, 20},
        .reward = 5,
        .damage = 1,
        .invisible = false,
        .splits_on_death = false,
        .split_count = 0
    },
    // ENEMY_TANK
    {
        .health = 15,
        .speed = 1.5f,
        .color = {50, 50, 200},
        .reward = 10,
        .damage = 3,
        .invisible = false,
        .splits_on_death = false,
        .split_count = 0
    },
    // ENEMY_SPLITTER
    {
        .health = 8,
        .speed = 2.0f,
        .color = {200, 200, 50},
        .reward = 8,
        .damage = 2,
        .invisible = false,
        .splits_on_death = true,
        .split_count = 2
    },
    // ENEMY_GHOST
    {
        .health = 5,
        .speed = 3.0f,
        .color = {150, 150, 255},
        .reward = 5,
        .damage = 1,
        .invisible = true,
        .splits_on_death = false,
        .split_count = 0
    }
};

// ============================================================================
// TOWER STATS TABLE
// ============================================================================

const TowerStats TOWER_STATS_TABLE[] = {
    // TOWER_MACHINE_GUN
    {
        .cost = 50,
        .damage = 1,
        .range = 8.0f,
        .fire_rate = 0.2f,
        .projectile_speed = 10.0f,
        .color = {255, 255, 0},
        .can_see_invisible = false,
        .is_radar = false,
        .splash_radius = 0
    },
    // TOWER_CANNON
    {
        .cost = 80,
        .damage = 4,
        .range = 7.0f,
        .fire_rate = 0.8f,
        .projectile_speed = 6.0f,
        .color = {255, 150, 0},
        .can_see_invisible = false,
        .is_radar = false,
        .splash_radius = 2
    },
    // TOWER_SNIPER
    {
        .cost = 100,
        .damage = 5,
        .range = 16.0f,
        .fire_rate = 1.5f,
        .projectile_speed = 20.0f,  // Fast but not instant
        .color = {200, 255, 200},
        .can_see_invisible = true,
        .is_radar = false,
        .splash_radius = 0
    },
    // TOWER_RADAR
    {
        .cost = 60,
        .damage = 0,
        .range = 10.0f,
        .fire_rate = 0.0f,
        .projectile_speed = 0.0f,
        .color = {0, 255, 255},
        .can_see_invisible = true,
        .is_radar = true,
        .splash_radius = 0
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

float distance_squared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

float distance(float x1, float y1, float x2, float y2) {
    return sqrtf(distance_squared(x1, y1, x2, y2));
}

bool is_in_range(float x1, float y1, float x2, float y2, float range) {
    return distance_squared(x1, y1, x2, y2) <= range * range;
}

// ============================================================================
// ENEMY IMPLEMENTATION
// ============================================================================

void enemy_init(Enemy* enemy, EnemyType type, float start_x, float start_y) {
    const EnemyStats* stats = &ENEMY_STATS_TABLE[type];

    enemy->x = start_x;
    enemy->y = start_y;
    enemy->speed = stats->speed;
    enemy->health = stats->health;
    enemy->max_health = stats->health;
    enemy->type = type;
    enemy->color = stats->color;
    enemy->path_index = 0;
    enemy->path_progress = 0.0f;
    enemy->alive = true;
    enemy->invisible = stats->invisible;
    enemy->revealed = !stats->invisible;
}

void enemy_update(Enemy* enemy, float dt, GameState* game) {
    if (!enemy->alive) return;

    if (enemy->path_index + 1 >= game->path_length) {
        // Reached end of path
        enemy->alive = false;
        const EnemyStats* stats = &ENEMY_STATS_TABLE[enemy->type];
        if (game->lives > stats->damage) {
            game->lives -= stats->damage;
        } else {
            game->lives = 0;
        }
        return;
    }

    // Current and next waypoint
    int16_t x1 = game->path[enemy->path_index].x;
    int16_t y1 = game->path[enemy->path_index].y;
    int16_t x2 = game->path[enemy->path_index + 1].x;
    int16_t y2 = game->path[enemy->path_index + 1].y;

    float dx = (float)(x2 - x1);
    float dy = (float)(y2 - y1);
    float segment_length = sqrtf(dx * dx + dy * dy);
    if (segment_length == 0.0f) segment_length = 0.0001f;

    float vx = dx / segment_length;
    float vy = dy / segment_length;

    float move_dist = enemy->speed * dt;
    enemy->x += vx * move_dist;
    enemy->y += vy * move_dist;
    enemy->path_progress += move_dist;

    float dist_to_next = distance(enemy->x, enemy->y, x2, y2);
    if (dist_to_next < 0.5f) {
        enemy->path_index++;
    }

    if (enemy->health <= 0) {
        enemy->alive = false;
    }
}

void enemy_draw(const Enemy* enemy) {
    if (!enemy->alive) return;

    int x = (int)enemy->x;
    int y = (int)enemy->y;

    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }

    // Ghost enemies are barely visible
    if (enemy->invisible && !enemy->revealed) {
        Color ghost_color = {enemy->color.r / 8, enemy->color.g / 8, enemy->color.b / 4};
        set_pixel(x, y, ghost_color);
    } else {
        set_pixel(x, y, enemy->color);
    }
}

// ============================================================================
// TOWER IMPLEMENTATION
// ============================================================================

void tower_init(Tower* tower, TowerType type, int16_t x, int16_t y) {
    const TowerStats* stats = &TOWER_STATS_TABLE[type];

    tower->x = (float)x;
    tower->y = (float)y;
    tower->type = type;
    tower->color = stats->color;

    tower->damage = stats->damage;
    tower->range = stats->range;
    tower->fire_rate = stats->fire_rate;
    tower->projectile_speed = stats->projectile_speed;
    tower->splash_radius = stats->splash_radius;

    tower->time_since_shot = 0.0f;
    tower->target_index = -1;

    tower->can_see_invisible = stats->can_see_invisible;
    tower->is_radar = stats->is_radar;
    tower->radar_angle = 0.0f;
}

void tower_shoot(Tower* tower, uint8_t target_index, GameState* game) {
    // Find free projectile slot
    if (game->projectile_count >= MAX_PROJECTILES) return;

    Projectile* proj = &game->projectiles[game->projectile_count];

    Color proj_color = Color{255, 255, 0};  // Yellow projectiles
    
    // Store POINTER to enemy (via target position) instead of index
    Enemy* target = &game->enemies[target_index];
    
    projectile_init(proj,
                    tower->x,
                    tower->y,
                    target->x,  // Target X position
                    target->y,  // Target Y position
                    tower->damage,
                    tower->projectile_speed,
                    proj_color,
                    tower->splash_radius);

    game->projectile_count++;
}

void tower_update(Tower* tower, float dt, GameState* game) {
    if (tower->is_radar) {
        tower->radar_angle += 2.0f * dt;  // 2 rad/s
        if (tower->radar_angle > 2.0f * 3.14159f) {
            tower->radar_angle -= 2.0f * 3.14159f;
        }

        // Reveal invisible enemies in range
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy* enemy = &game->enemies[i];
            if (!enemy->alive) continue;
            if (!enemy->invisible) continue;

            if (is_in_range(tower->x, tower->y, enemy->x, enemy->y, tower->range)) {
                enemy->revealed = true;
            }
        }

        return;
    }

    tower->time_since_shot += dt;

    if (tower->time_since_shot < tower->fire_rate) {
        return;
    }

    int best_index = -1;
    float best_progress = -1.0f;

    for (int i = 0; i < game->enemy_count; i++) {
        Enemy* e = &game->enemies[i];
        if (!e->alive) continue;
        if (e->invisible && !tower->can_see_invisible && !e->revealed) continue;

        if (is_in_range(tower->x, tower->y, e->x, e->y, tower->range)) {
            if (e->path_progress > best_progress) {
                best_progress = e->path_progress;
                best_index = i;
            }
        }
    }

    if (best_index != -1) {
        tower->time_since_shot = 0.0f;
        tower_shoot(tower, (uint8_t)best_index, game);
    }
}

void draw_tower_range(int16_t x, int16_t y, float range) {
    // Draw a circle showing the tower's range
    // Using midpoint circle algorithm
    int cx = x;
    int cy = y;
    int radius = (int)range;
    
    int dx = radius;
    int dy = 0;
    int err = 0;
    
    Color range_color = {80, 80, 80};  // Gray range indicator
    
    while (dx >= dy) {
        // Draw 8 symmetric points
        int points[8][2] = {
            {cx + dx, cy + dy}, {cx + dy, cy + dx},
            {cx - dy, cy + dx}, {cx - dx, cy + dy},
            {cx - dx, cy - dy}, {cx - dy, cy - dx},
            {cx + dy, cy - dx}, {cx + dx, cy - dy}
        };
        
        for (int i = 0; i < 8; i++) {
            int px = points[i][0];
            int py = points[i][1];
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                set_pixel(px, py, range_color);
            }
        }
        
        if (err <= 0) {
            dy += 1;
            err += 2 * dy + 1;
        }
        
        if (err > 0) {
            dx -= 1;
            err -= 2 * dx + 1;
        }
    }
}

// ============================================================================
// PROJECTILE IMPLEMENTATION - FIXED VERSION
// ============================================================================

void projectile_init(Projectile* proj, float x, float y, 
                     float target_x, float target_y,
                     uint8_t damage, float speed, Color color, uint8_t splash) {
    proj->x = x;
    proj->y = y;
    proj->target_x = target_x;  // Store target position
    proj->target_y = target_y;
    proj->damage = damage;
    proj->speed = speed;
    proj->color = color;
    proj->splash_radius = splash;
    proj->active = true;
    
    // Calculate direction to target
    float dx = target_x - x;
    float dy = target_y - y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist > 0.0f) {
        proj->vx = (dx / dist) * speed;
        proj->vy = (dy / dist) * speed;
    } else {
        proj->vx = 0.0f;
        proj->vy = 0.0f;
    }
}

bool projectile_update(Projectile* proj, float dt, GameState* game) {
    if (!proj->active) return true;

    // Move projectile
    proj->x += proj->vx * dt;
    proj->y += proj->vy * dt;

    // Check if projectile hit any enemy
    Enemy* hit_enemy = NULL;
    int hit_index = -1;
    float closest_dist = 1.0f;  // Hit radius
    
    for (int i = 0; i < game->enemy_count; i++) {
        Enemy* e = &game->enemies[i];
        if (!e->alive) continue;
        
        float dist = distance(proj->x, proj->y, e->x, e->y);
        if (dist < closest_dist) {
            closest_dist = dist;
            hit_enemy = e;
            hit_index = i;
        }
    }

    // If we hit an enemy
    if (hit_enemy != NULL) {
        // Damage the target
        hit_enemy->health -= proj->damage;
        
        printf("HIT! Enemy %d took %d damage (HP: %d/%d)\n", 
               hit_index, proj->damage, hit_enemy->health, hit_enemy->max_health);
        
        if (hit_enemy->health <= 0) {
            hit_enemy->alive = false;
            const EnemyStats* stats = &ENEMY_STATS_TABLE[hit_enemy->type];
            game->money += stats->reward;
            game->score += stats->reward * 10;
            printf("KILL! +$%d +%d score\n", stats->reward, stats->reward * 10);
        }

        // Splash damage
        if (proj->splash_radius > 0) {
            float splash_r = (float)proj->splash_radius;
            for (int i = 0; i < game->enemy_count; i++) {
                if (i == hit_index) continue;  // Already damaged
                Enemy* e = &game->enemies[i];
                if (!e->alive) continue;
                
                if (distance(proj->x, proj->y, e->x, e->y) <= splash_r) {
                    e->health -= proj->damage;
                    printf("SPLASH! Enemy %d took %d damage\n", i, proj->damage);
                    
                    if (e->health <= 0) {
                        e->alive = false;
                        const EnemyStats* stats = &ENEMY_STATS_TABLE[e->type];
                        game->money += stats->reward;
                        game->score += stats->reward * 10;
                    }
                }
            }
        }

        proj->active = false;
        return true;
    }

    // Check if projectile is out of bounds
    if (proj->x < -5 || proj->x > MATRIX_WIDTH + 5 ||
        proj->y < -5 || proj->y > MATRIX_HEIGHT + 5) {
        proj->active = false;
        return true;
    }

    return true;
}

void projectile_draw(const Projectile* proj) {
    if (!proj->active) return;

    int x = (int)proj->x;
    int y = (int)proj->y;

    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
        set_pixel(x, y, proj->color);
    }
}

// ============================================================================
// GAME IMPLEMENTATION
// ============================================================================

void game_init(GameState* game) {
    // Zero-initialize all fields properly
    game->enemy_count = 0;
    game->tower_count = 0;
    game->projectile_count = 0;
    game->path_length = 0;
    game->tower_slot_count = 0;

    game->money = 200;
    game->lives = 20;
    game->score = 0;
    game->game_time = 0.0f;
    game->wave_number = 0;
    game->total_waves = 6;

    // Initialize simple path (right to left)
    game->path[0] = {63, 15};
    game->path[1] = {50, 15};
    game->path[2] = {50, 25};
    game->path[3] = {30, 25};
    game->path[4] = {30, 10};
    game->path[5] = {15, 10};
    game->path[6] = {15, 20};
    game->path[7] = {0, 20};
    game->path_length = 8;

    // Initialize tower slots
    game->tower_slots[0] = {55, 8, false};
    game->tower_slots[1] = {55, 22, false};
    game->tower_slots[2] = {38, 18, false};
    game->tower_slots[3] = {20, 6, false};
    game->tower_slots[4] = {20, 28, false};
    game->tower_slot_count = 5;
}

void game_spawn_enemy(GameState* game, EnemyType type) {
    if (game->enemy_count >= MAX_ENEMIES) return;

    Enemy* enemy = &game->enemies[game->enemy_count];
    int16_t start_x = game->path[0].x;
    int16_t start_y = game->path[0].y;
    enemy_init(enemy, type, (float)start_x, (float)start_y);

    game->enemy_count++;
}

bool game_place_tower(GameState* game, TowerType type, int16_t x, int16_t y) {
    if (game->tower_count >= MAX_TOWERS) return false;

    const TowerStats* stats = &TOWER_STATS_TABLE[type];
    if (game->money < stats->cost) return false;

    int slot_index = -1;
    for (int i = 0; i < game->tower_slot_count; i++) {
        if (!game->tower_slots[i].occupied &&
            game->tower_slots[i].x == x &&
            game->tower_slots[i].y == y) {
            slot_index = i;
            break;
        }
    }

    if (slot_index == -1) {
        return false;
    }

    Tower* tower = &game->towers[game->tower_count];
    tower_init(tower, type, x, y);

    game->tower_slots[slot_index].occupied = true;
    game->tower_count++;
    game->money -= stats->cost;
    return true;
}

void game_update(GameState* game, float dt) {
    game->game_time += dt;

    // Update towers (they shoot projectiles)
    for (int i = 0; i < game->tower_count; i++) {
        tower_update(&game->towers[i], dt, game);
    }

    // Update projectiles (they move and check for hits)
    for (int i = 0; i < game->projectile_count; i++) {
        Projectile* proj = &game->projectiles[i];
        if (!proj->active) continue;
        projectile_update(proj, dt, game);
    }

    // Remove inactive projectiles
    int write_index = 0;
    for (int i = 0; i < game->projectile_count; i++) {
        if (game->projectiles[i].active) {
            if (write_index != i) {
                game->projectiles[write_index] = game->projectiles[i];
            }
            write_index++;
        }
    }
    game->projectile_count = write_index;

    // Update enemies (they move along path)
    for (int i = 0; i < game->enemy_count; i++) {
        Enemy* e = &game->enemies[i];
        if (!e->alive) continue;
        enemy_update(e, dt, game);
    }

    // Remove dead enemies
    write_index = 0;
    for (int i = 0; i < game->enemy_count; i++) {
        Enemy* e = &game->enemies[i];
        if (e->alive) {
            if (write_index != i) {
                game->enemies[write_index] = *e;
            }
            write_index++;
        }
    }
    game->enemy_count = write_index;
}