#include "wave_system.h"
#include <stdio.h>

// ============================================================================
// WAVE DEFINITIONS (3 demo waves)
// ============================================================================

// Wave 1: "Scout Swarm" - Easy introduction
static const WaveSpawn wave1_spawns[] = {
    {ENEMY_SCOUT, 0.0f},
    {ENEMY_SCOUT, 1.0f},
    {ENEMY_SCOUT, 2.0f},
    {ENEMY_SCOUT, 3.0f},
    {ENEMY_SCOUT, 4.0f},
    {ENEMY_SCOUT, 5.0f}
};

// Wave 2: "Mixed Assault" - Scouts and tanks
static const WaveSpawn wave2_spawns[] = {
    {ENEMY_SCOUT, 0.0f},
    {ENEMY_SCOUT, 0.5f},
    {ENEMY_TANK, 1.5f},
    {ENEMY_SCOUT, 2.5f},
    {ENEMY_SCOUT, 3.0f},
    {ENEMY_TANK, 4.0f},
    {ENEMY_SCOUT, 5.0f},
    {ENEMY_TANK, 6.5f},
    {ENEMY_SCOUT, 7.5f},
    {ENEMY_SCOUT, 8.0f}
};

// Wave 3: "Special Forces" - All enemy types
static const WaveSpawn wave3_spawns[] = {
    {ENEMY_SCOUT, 0.0f},
    {ENEMY_GHOST, 1.0f},
    {ENEMY_SCOUT, 2.0f},
    {ENEMY_SPLITTER, 3.0f},
    {ENEMY_TANK, 4.0f},
    {ENEMY_GHOST, 5.0f},
    {ENEMY_SCOUT, 6.0f},
    {ENEMY_SPLITTER, 7.0f},
    {ENEMY_TANK, 8.0f},
    {ENEMY_GHOST, 9.0f},
    {ENEMY_SPLITTER, 10.0f},
    {ENEMY_TANK, 11.5f},
    {ENEMY_SCOUT, 12.5f},
    {ENEMY_SCOUT, 13.0f}
};

// Wave table
static const WaveDef WAVE_TABLE[] = {
    {wave1_spawns, sizeof(wave1_spawns) / sizeof(WaveSpawn), "Scout Swarm"},
    {wave2_spawns, sizeof(wave2_spawns) / sizeof(WaveSpawn), "Mixed Assault"},
    {wave3_spawns, sizeof(wave3_spawns) / sizeof(WaveSpawn), "Special Forces"}
};

#define TOTAL_WAVES (sizeof(WAVE_TABLE) / sizeof(WaveDef))

// ============================================================================
// WAVE MANAGER IMPLEMENTATION
// ============================================================================

void wave_manager_init(WaveManager* wm) {
    wm->wave_timer = 0.0f;
    wm->current_wave = 0;
    wm->spawns_completed = 0;
    wm->wave_active = false;
    wm->wave_complete = false;
    wm->wave_complete_timer = 0.0f;
}

void wave_manager_start_wave(WaveManager* wm, uint8_t wave_number, GameState* game) {
    if (wave_number >= TOTAL_WAVES) {
        printf("ERROR: Invalid wave number %d (max %d)\n", wave_number, TOTAL_WAVES - 1);
        return;
    }
    
    wm->current_wave = wave_number;
    wm->wave_timer = 0.0f;
    wm->spawns_completed = 0;
    wm->wave_active = true;
    wm->wave_complete = false;
    wm->wave_complete_timer = 0.0f;
    
    const WaveDef* wave = &WAVE_TABLE[wave_number];
    printf("\n=== WAVE %d: %s ===\n", wave_number + 1, wave->name);
    printf("Enemies: %d\n", wave->spawn_count);
    printf("=====================\n\n");
}

void wave_manager_update(WaveManager* wm, float dt, GameState* game) {
    if (!wm->wave_active) return;
    
    // Update wave timer
    wm->wave_timer += dt;
    
    const WaveDef* wave = &WAVE_TABLE[wm->current_wave];
    
    // Check if we should spawn any enemies
    while (wm->spawns_completed < wave->spawn_count) {
        const WaveSpawn* spawn = &wave->spawns[wm->spawns_completed];
        
        // Check if it's time to spawn this enemy
        if (wm->wave_timer >= spawn->spawn_time) {
            game_spawn_enemy(game, spawn->type);
            wm->spawns_completed++;
            printf("Spawned enemy %d/%d (type %d) at %.1fs\n", 
                   wm->spawns_completed, wave->spawn_count, spawn->type, wm->wave_timer);
        } else {
            // Not time yet, break out of loop
            break;
        }
    }
    
    // Check if all enemies have been spawned
    if (wm->spawns_completed >= wave->spawn_count && !wm->wave_complete) {
        wm->wave_complete = true;
        wm->wave_complete_timer = 0.0f;
        printf("All enemies spawned for wave %d!\n", wm->current_wave + 1);
    }
    
    // Update completion timer
    if (wm->wave_complete) {
        wm->wave_complete_timer += dt;
    }
}

bool wave_manager_is_complete(const WaveManager* wm, const GameState* game) {
    // Wave is complete when:
    // 1. All enemies have been spawned (wave_complete = true)
    // 2. All enemies have been defeated (enemy_count = 0)
    // 3. A short delay has passed (0.5s) to allow for split enemies
    
    if (!wm->wave_complete) return false;
    if (game->enemy_count > 0) return false;
    if (wm->wave_complete_timer < 0.5f) return false;
    
    return true;
}

uint8_t wave_manager_get_total_waves() {
    return TOTAL_WAVES;
}

const WaveDef* wave_manager_get_wave(uint8_t wave_number) {
    if (wave_number >= TOTAL_WAVES) return NULL;
    return &WAVE_TABLE[wave_number];
}
