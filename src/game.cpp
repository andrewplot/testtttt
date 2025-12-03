// game.cpp - Core game implementation
#include "game_types.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "matrix.hh"

// ============================================================================
// ENEMY STATS TABLE (replaces Python ENEMY_TYPES dict)
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
        .projectile_speed = 999.0f,  // instant-ish
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
        //framebuffer[y][x] = ghost_color;
        set_pixel(x, y, ghost_color);
    } else {
        //framebuffer[y][x] = enemy->color;
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
    projectile_init(proj,
                    tower->x,
                    tower->y,
                    target_index,
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
        if (e->invisible && !tower->can_see_invisible) continue;

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

void tower_draw(const Tower* tower) {
    int x = (int)tower->x;
    int y = (int)tower->y;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                // framebuffer[py][px] = tower->color;
                set_pixel(px, py, tower->color);
            }
        }
    }

    if (tower->is_radar) {
        int tip_x = x + (int)(cosf(tower->radar_angle) * 3.0f);
        int tip_y = y + (int)(sinf(tower->radar_angle) * 3.0f);
        if (tip_x >= 0 && tip_x < MATRIX_WIDTH && tip_y >= 0 && tip_y < MATRIX_HEIGHT) {
            //framebuffer[tip_y][tip_x] = Color{0, 255, 255};
            set_pixel(tip_x, tip_y, Color(0, 255, 255));

        }
    }
}

// ============================================================================
// PROJECTILE IMPLEMENTATION
// ============================================================================

void projectile_init(Projectile* proj, float x, float y, uint8_t target_idx,
                     uint8_t damage, float speed, Color color, uint8_t splash) {
    proj->x = x;
    proj->y = y;
    proj->target_index = target_idx;
    proj->damage = damage;
    proj->speed = speed;
    proj->color = color;
    proj->splash_radius = splash;
    proj->active = true;
}

bool projectile_update(Projectile* proj, float dt, GameState* game) {
    if (!proj->active) return true;

    Enemy* target = &game->enemies[proj->target_index];

    if (!target->alive) {
        proj->active = false;
        return true;
    }

    float dx = target->x - proj->x;
    float dy = target->y - proj->y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist < 0.3f) {
        target->health -= proj->damage;
        if (target->health <= 0) {
            target->alive = false;
            const EnemyStats* stats = &ENEMY_STATS_TABLE[target->type];
            game->money += stats->reward;
            game->score += stats->reward * 10;
        }

        if (proj->splash_radius > 0) {
            float splash_r = (float)proj->splash_radius;
            for (int i = 0; i < game->enemy_count; i++) {
                if (i == proj->target_index) continue;
                Enemy* e = &game->enemies[i];
                if (!e->alive) continue;
                if (distance(proj->x, proj->y, e->x, e->y) <= splash_r) {
                    e->health -= proj->damage;
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

    if (proj->speed <= 0.0f) proj->speed = 10.0f;

    float vx = dx / dist;
    float vy = dy / dist;

    proj->x += vx * proj->speed * dt;
    proj->y += vy * proj->speed * dt;

    return true;
}

void projectile_draw(const Projectile* proj) {
    if (!proj->active) return;

    int x = (int)proj->x;
    int y = (int)proj->y;

    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
        //framebuffer[y][x] = proj->color;
        set_pixel(x, y, proj->color);
    }
}

// ============================================================================
// GAME IMPLEMENTATION
// ============================================================================

void game_init(GameState* game) {
    memset(game, 0, sizeof(GameState));

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

    for (int i = 0; i < game->tower_count; i++) {
        tower_update(&game->towers[i], dt, game);
    }

    for (int i = 0; i < game->projectile_count; i++) {
        Projectile* proj = &game->projectiles[i];
        if (!proj->active) continue;
        projectile_update(proj, dt, game);
    }

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

    write_index = 0;
    for (int i = 0; i < game->enemy_count; i++) {
        Enemy* e = &game->enemies[i];
        if (!e->alive) continue;
        enemy_update(e, dt, game);
        if (e->alive) {
            if (write_index != i) {
                game->enemies[write_index] = *e;
            }
            write_index++;
        }
    }
    game->enemy_count = write_index;
}

void game_draw(const GameState* game) {
    // Draw path
    for (int i = 0; i < game->path_length; i++) {
        int x = game->path[i].x;
        int y = game->path[i].y;
        if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
            //framebuffer[y][x] = Color{100, 100, 100};
            set_pixel(x, y, Color(100,100,100));
        }
    }

    // Draw tower slots
    for (int i = 0; i < game->tower_slot_count; i++) {
        int x = game->tower_slots[i].x;
        int y = game->tower_slots[i].y;
        Color slot_color = game->tower_slots[i].occupied ?
                           Color{60, 50, 0} : Color{128, 107, 0};

        for (int dy = -2; dy < 2; dy++) {
            for (int dx = -2; dx < 2; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < MATRIX_WIDTH && py >= 0 && py < MATRIX_HEIGHT) {
                    framebuffer[py][px] = slot_color;
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
