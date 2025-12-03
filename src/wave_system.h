#ifndef WAVE_SYSTEM_H
#define WAVE_SYSTEM_H

#include "game_types.h"

// Wave spawn entry - defines what enemy to spawn and when
typedef struct {
    EnemyType type;
    float spawn_time;  // Time in seconds from wave start
} WaveSpawn;

// Wave definition
typedef struct {
    const WaveSpawn* spawns;
    uint8_t spawn_count;
    const char* name;
} WaveDef;

// Wave manager state
typedef struct {
    float wave_timer;           // Time since wave started
    uint8_t current_wave;       // Current wave number (0-based)
    uint8_t spawns_completed;   // How many enemies spawned so far
    bool wave_active;           // Is a wave currently running?
    bool wave_complete;         // Did we finish all spawns?
    float wave_complete_timer;  // Time since last enemy spawned
} WaveManager;

// Initialize wave manager
void wave_manager_init(WaveManager* wm);

// Start a wave
void wave_manager_start_wave(WaveManager* wm, uint8_t wave_number, GameState* game);

// Update wave manager (spawns enemies at appropriate times)
void wave_manager_update(WaveManager* wm, float dt, GameState* game);

// Check if wave is complete (all enemies spawned AND defeated)
bool wave_manager_is_complete(const WaveManager* wm, const GameState* game);

// Get total number of waves
uint8_t wave_manager_get_total_waves();

// Get wave definition
const WaveDef* wave_manager_get_wave(uint8_t wave_number);

#endif // WAVE_SYSTEM_H
