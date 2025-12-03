
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "game_types.h"
#include "map_render.hh"
#include "joystick.hh"
#include "oled_display.hh"
#include "buzzer_pwm.hh"
#include "rfid_bridge.hh"
#include "rfid.hh"
#include "pin-definitions.hh"
#include "wave_system.h"  // NEW: Wave system


/*
4. wave system
5. start sequence + banner planes + death sequence
6. OPTIMIZATIONS
7. place towers anywhere!
8. make towers rotate and track
9. abilities
*/

// Forward declarations for LED matrix driver functions
void init_matrix();
void swap_frames();
void render_frame();
void set_pixel(int x, int y, Color color);

// Global game data
GameState game;
WaveManager wave_manager;  // NEW: Wave manager
Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

uint32_t last_time_ms = 0;

// Cursor over tower slots
int current_slot_index = 0;
bool show_placement_mode = false;

// Track last scanned tower to avoid repeated triggers
extern TowerType scanned_tower;
TowerType last_scanned_tower = TOWER_BLANK;

// Initialize everything
static void setup_hardware() {
    stdio_init_all();

    // Matrix, joystick, OLED, RFID, buzzer
    init_matrix();
    init_joystick();
    rfid_setup();
    init_oled();

    buzzer_pwm_init();
    buzzer_set_volume(40);

    // Initialize game
    game_init(&game);
    game.selected_tower = TOWER_MACHINE_GUN;
    
    // Initialize map rendering
    map_render_init(&game);

    // NEW: Initialize wave manager
    wave_manager_init(&wave_manager);
    
    // NEW: Start wave 1 automatically
    wave_manager_start_wave(&wave_manager, 0, &game);
    start_sound();

    last_time_ms = to_ms_since_boot(get_absolute_time());
}

// Check if RFID selected a new tower and enter placement mode
static TowerType convert_hw_to_game_tower(HardwareTowerType hw) {
    switch (hw) {
        case MACHINE_GUN: return TOWER_MACHINE_GUN;
        case CANNON:      return TOWER_CANNON;
        case SNIPER:      return TOWER_SNIPER;
        case RADAR:       return TOWER_RADAR;
        case BLANK:
        default:          return TOWER_BLANK;
    }
}

static void check_tower_selection() {
    if (!rfid_flag) return;
    victory_sound();
    
    rfid_flag = false;
    
    HardwareTowerType hw_tower_scanned = sample_rfid();
    TowerType game_tower = convert_hw_to_game_tower(hw_tower_scanned);
    
    if (game_tower != TOWER_BLANK && game_tower != last_scanned_tower) {
        printf("=== NEW TOWER SCANNED: Hardware=%d, Game=%d ===\n", hw_tower_scanned, game_tower);
        
        game.selected_tower = game_tower;
        scanned_tower = game_tower;
        
        show_placement_mode = true;
        current_slot_index = 0;
        
        const TowerStats* stats = &TOWER_STATS_TABLE[game_tower];
        printf("Selected tower - Cost: %d, Range: %.1f, Damage: %d\n",
               stats->cost, stats->range, stats->damage);
        
        last_scanned_tower = game_tower;
    }
}

// Joystick → choose tower slot index
static void handle_joystick() {
    if (!joystick_flag) return;
    joystick_flag = false;
    
    JoystickDirection jx = sample_js_x();
    bool sel = sample_js_select();

    // Navigation (X-axis)
    static JoystickDirection last_jx = center;
    
    if (show_placement_mode && game.tower_slot_count > 0 && jx != last_jx) {
        if (jx == right) {
            current_slot_index++;
            if (current_slot_index >= game.tower_slot_count)
                current_slot_index = 0;
            
            int attempts = 0;
            while (game.tower_slots[current_slot_index].occupied && attempts < game.tower_slot_count) {
                current_slot_index++;
                if (current_slot_index >= game.tower_slot_count)
                    current_slot_index = 0;
                attempts++;
            }
            printf("→ Slot %d\n", current_slot_index);
        } 
        else if (jx == left) {
            current_slot_index--;
            if (current_slot_index < 0)
                current_slot_index = game.tower_slot_count - 1;
            
            int attempts = 0;
            while (game.tower_slots[current_slot_index].occupied && attempts < game.tower_slot_count) {
                current_slot_index--;
                if (current_slot_index < 0)
                    current_slot_index = game.tower_slot_count - 1;
                attempts++;
            }
            printf("← Slot %d\n", current_slot_index);
        }
    }
    last_jx = jx;

    // Button
    static bool last_sel = false;
    
    if (sel != last_sel) {
        if (sel) {
            printf("=== BUTTON CLICK ===\n");
            
            if (show_placement_mode && game.tower_slot_count > 0) {
                TowerSlot* slot = &game.tower_slots[current_slot_index];
                
                if (!slot->occupied) {
                    bool ok = game_place_tower(&game, game.selected_tower, slot->x, slot->y);
                    if (ok) {
                        slot->occupied = true;
                        beep_ok();
                        show_placement_mode = false;
                        printf("✓ PLACED!\n");
                    } else {
                        error_sound();
                    }
                } else {
                    error_sound();
                }
            }
        }
        last_sel = sel;
    }
}

// Update game logic - NEW VERSION WITH WAVE SYSTEM
static void update_game() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    float dt = (now - last_time_ms) / 1000.0f;
    last_time_ms = now;
    if (dt > 0.1f) dt = 0.1f;

    game.game_time += dt;

    // NEW: Update wave manager (spawns enemies at scheduled times)
    wave_manager_update(&wave_manager, dt, &game);
    
    // NEW: Check if wave is complete
    static bool wave_just_completed = false;
    if (wave_manager_is_complete(&wave_manager, &game)) {
        if (!wave_just_completed) {
            wave_just_completed = true;
            
            printf("\n*** WAVE %d COMPLETE! ***\n", wave_manager.current_wave + 1);
            victory_sound();
            
            // Check if there are more waves
            if (wave_manager.current_wave + 1 < wave_manager_get_total_waves()) {
                printf("Next wave starting in 3 seconds...\n\n");
                sleep_ms(3000);
                
                // Start next wave
                wave_manager_start_wave(&wave_manager, wave_manager.current_wave + 1, &game);
                start_sound();
                wave_just_completed = false;
            } else {
                printf("\n*** ALL WAVES COMPLETE! VICTORY! ***\n");
                printf("Final Score: %d\n", game.score);
                printf("Money Remaining: %d\n", game.money);
                printf("Lives Remaining: %d\n", game.lives);
                printf("================================\n\n");
                
                // Game complete - restart from wave 1
                sleep_ms(5000);
                wave_manager_start_wave(&wave_manager, 0, &game);
                start_sound();
                wave_just_completed = false;
            }
        }
    } else {
        wave_just_completed = false;
    }

    // Update game logic
    game_update(&game, dt);
}

// Render game into framebuffer, then to LED matrix + OLED
static void render_game_to_framebuffer() {
    // Draw map (background + path with textures)
    map_render_draw(&game);
    
    // Draw decorations (trees, rocks, lakes) - BEFORE game objects
    map_render_decorations();

    // Draw game objects (tower slots, enemies, towers, projectiles)
    game_draw(&game);

    // If in placement mode, show range indicator at current slot
    if (show_placement_mode && game.tower_slot_count > 0) {
        TowerSlot* slot = &game.tower_slots[current_slot_index];
        
        const TowerStats* stats = &TOWER_STATS_TABLE[game.selected_tower];
        draw_tower_range(slot->x, slot->y, stats->range);
        
        bool blink_on = ((int)(game.game_time * 2.0f) % 2) == 0;
        
        if (blink_on) {
            int cx = slot->x;
            int cy = slot->y;
            
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    if (abs(dx) == 2 || abs(dy) == 2) {
                        int px = cx + dx;
                        int py = cy + dy;
                        if (px >= 0 && px < MATRIX_WIDTH &&
                            py >= 0 && py < MATRIX_HEIGHT) {
                            set_pixel(px, py, Color(100, 100, 255));
                        }
                    }
                }
            }
        }
    }
}

// NEW: Updated OLED UI to show wave information
static void render_oled_ui() {
    char line1[17];
    char line2[17];

    // Line 1: Money and wave number
    snprintf(line1, sizeof(line1), "$%d W%d/%d", 
             game.money, 
             wave_manager.current_wave + 1,
             wave_manager_get_total_waves());
    
    // Line 2: Lives and score
    snprintf(line2, sizeof(line2), "HP:%d S:%d", 
             game.lives,
             game.score);

    oled_print(line1, line2);
}

// Core 1 rendering
void render_matrix() {
    for (;;) {
        render_frame();
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking();
            swap_frames();
        }
    }
}

// Main
int main() {
    setup_hardware();
    multicore_launch_core1(render_matrix);

    printf("\n=== TOWER DEFENSE GAME STARTED ===\n");
    printf("Instructions:\n");
    printf("1. Scan RFID tag to select tower type\n");
    printf("2. Use joystick LEFT/RIGHT to choose slot\n");
    printf("3. Press joystick SELECT button to place tower\n");
    printf("================================\n\n");

    while (true) {
        check_tower_selection();
        handle_joystick();
        
        update_game();
        render_game_to_framebuffer();
        render_oled_ui();
        multicore_fifo_push_blocking(1);

        sleep_ms(60);
    }

    return 0;
}