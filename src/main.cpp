// main.cpp – RP2350 + PicoSDK + PlatformIO tower defense game

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
#include "rfid.hh"  // For sample_rfid() direct call
#include "pin-definitions.hh"


/*

1. fix tower selection + tower ranges

2. brighten background and reduce noise + fix enemy alignment on horizontal path

3. tower sprites + slot sprites

4. wave system

5. start sequence + banner planes + death sequence

6. make decorations nicer

7. place towers anywhere!

8. make towers rotate and track

9. abilities

*/




// -----------------------------------------------------------------------------
// Forward declarations for LED matrix driver functions
// -----------------------------------------------------------------------------

void init_matrix();
void swap_frames();
void render_frame();
void set_pixel(int x, int y, Color color);

// -----------------------------------------------------------------------------
// Global game data
// -----------------------------------------------------------------------------

GameState game;
Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

uint32_t last_time_ms = 0;

// Cursor over tower slots
int current_slot_index = 0;
bool show_placement_mode = false;  // Track if user is in tower placement mode

// Track last scanned tower to avoid repeated triggers
extern TowerType scanned_tower;  // Defined in rfid_bridge.cpp
TowerType last_scanned_tower = TOWER_BLANK;

// -----------------------------------------------------------------------------
// Initialize everything
// -----------------------------------------------------------------------------

static void setup_hardware() {
    stdio_init_all();

    // Matrix, joystick, OLED, RFID, buzzer
    init_matrix();
    init_joystick();
    rfid_setup();
    init_oled();

    // buzzer on whatever pin your board uses (from pin-definitions.hh)
    buzzer_pwm_init();
    buzzer_set_volume(40);  // 40% duty

    // Initialize game
    game_init(&game);
    game.selected_tower = TOWER_MACHINE_GUN;
    
    // Initialize map rendering (generates textures)
    map_render_init(&game);

    // Spawn one test enemy so you see something move
    game_spawn_enemy(&game, ENEMY_SCOUT);

    last_time_ms = to_ms_since_boot(get_absolute_time());
}

// -----------------------------------------------------------------------------
// Check if RFID selected a new tower and enter placement mode
// -----------------------------------------------------------------------------

// Helper to convert hardware to game tower type
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
    // Only check when RFID flag is set (timer-based, every 1 second)
    if (!rfid_flag) return;
    
    rfid_flag = false;  // Clear the flag
    
    // Sample RFID once per timer period (returns HardwareTowerType)
    HardwareTowerType hw_tower_scanned = sample_rfid();
    
    // Convert to game TowerType
    TowerType game_tower = convert_hw_to_game_tower(hw_tower_scanned);
    
    // Check if a new tower was scanned (not BLANK and different from last)
    if (game_tower != TOWER_BLANK && game_tower != last_scanned_tower) {
        printf("=== NEW TOWER SCANNED: Hardware=%d, Game=%d ===\n", hw_tower_scanned, game_tower);
        
        // Update selected tower type and scanned_tower global
        game.selected_tower = game_tower;
        scanned_tower = game_tower;
        
        // Enter placement mode
        show_placement_mode = true;
        
        // Reset cursor to first available slot
        current_slot_index = 0;
        
        // Print tower info
        const TowerStats* stats = &TOWER_STATS_TABLE[game_tower];
        printf("Selected tower - Cost: %d, Range: %.1f, Damage: %d\n",
               stats->cost, stats->range, stats->damage);
        
        last_scanned_tower = game_tower;
    }
}

// -----------------------------------------------------------------------------
// Joystick → choose tower slot index
// -----------------------------------------------------------------------------

static void handle_joystick() {
    // Only check joystick when the flag is set (timer-based, every 25ms)
    if (!joystick_flag) return;
    
    joystick_flag = false;  // Clear the flag
    
    JoystickDirection jx = sample_js_x();
    JoystickDirection jy = sample_js_y();  // Read but ignore Y-axis
    bool sel = sample_js_select();

    // Debug - print when values change
    static JoystickDirection last_debug_jx = center;
    static JoystickDirection last_debug_jy = center;
    static bool last_debug_sel = false;
    
    if (jx != last_debug_jx || jy != last_debug_jy || sel != last_debug_sel) {
        printf("[JS] X=%d, Y=%d, SEL=%d\n", jx, jy, sel);
        last_debug_jx = jx;
        last_debug_jy = jy;
        last_debug_sel = sel;
    }

    // Only navigate if in placement mode
    if (show_placement_mode && game.tower_slot_count > 0) {
        // Navigate between slots using LEFT/RIGHT only (ignore Y-axis)
        static JoystickDirection last_jx = center;
        
        // Only trigger on edge (transition from center to left/right)
        if (jx == right && last_jx == center) {
            current_slot_index++;
            if (current_slot_index >= game.tower_slot_count)
                current_slot_index = 0;
            
            // Skip occupied slots
            int attempts = 0;
            while (game.tower_slots[current_slot_index].occupied && attempts < game.tower_slot_count) {
                current_slot_index++;
                if (current_slot_index >= game.tower_slot_count)
                    current_slot_index = 0;
                attempts++;
            }
            
            printf("→ RIGHT: Moved to slot %d at (%d, %d)\n", 
                   current_slot_index,
                   game.tower_slots[current_slot_index].x,
                   game.tower_slots[current_slot_index].y);
        } 
        else if (jx == left && last_jx == center) {
            current_slot_index--;
            if (current_slot_index < 0)
                current_slot_index = game.tower_slot_count - 1;
            
            // Skip occupied slots
            int attempts = 0;
            while (game.tower_slots[current_slot_index].occupied && attempts < game.tower_slot_count) {
                current_slot_index--;
                if (current_slot_index < 0)
                    current_slot_index = game.tower_slot_count - 1;
                attempts++;
            }
            
            printf("← LEFT: Moved to slot %d at (%d, %d)\n", 
                   current_slot_index,
                   game.tower_slots[current_slot_index].x,
                   game.tower_slots[current_slot_index].y);
        }
        
        last_jx = jx;
    }

    // On button press (SELECT), place tower at current slot
    // IMPORTANT: Only trigger on button press, NOT on Y-axis movement
    static bool last_sel = false;
    
    // Edge detection: transition from false to true (button just pressed)
    // AND make sure we're not also moving on X or Y axis (to avoid accidental triggers)
    if (sel && !last_sel && jx == center && jy == center) {
        printf("=== SELECT BUTTON PRESSED (clean press) ===\n");
        
        if (show_placement_mode && game.tower_slot_count > 0) {
            printf("=== ATTEMPTING TOWER PLACEMENT ===\n");
            TowerSlot* slot = &game.tower_slots[current_slot_index];
            
            printf("Slot %d: pos=(%d,%d), occupied=%d\n",
                   current_slot_index, slot->x, slot->y, slot->occupied);
            printf("Selected tower: %d, Cost: %d, Money: %d\n",
                   game.selected_tower, TOWER_STATS_TABLE[game.selected_tower].cost, game.money);

            if (!slot->occupied) {
                bool ok = game_place_tower(&game,
                                           game.selected_tower,
                                           slot->x,
                                           slot->y);
                if (ok) {
                    slot->occupied = true;
                    beep_ok();
                    show_placement_mode = false;  // Exit placement mode after placing
                    printf("✓ TOWER PLACED SUCCESSFULLY!\n");
                    printf("Remaining money: %d, Tower count: %d\n", game.money, game.tower_count);
                } else {
                    error_sound();
                    printf("✗ PLACEMENT FAILED (insufficient funds)\n");
                }
            } else {
                error_sound();
                printf("✗ SLOT ALREADY OCCUPIED\n");
            }
        } else {
            printf("✗ Not in placement mode - scan RFID tag first\n");
        }
    } else if (sel && !last_sel) {
        printf("Button pressed but joystick not centered (X=%d, Y=%d) - ignoring\n", jx, jy);
    }
    
    last_sel = sel;
}

// -----------------------------------------------------------------------------
// Update game logic
// -----------------------------------------------------------------------------

static void update_game() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    float dt = (now - last_time_ms) / 1000.0f;
    last_time_ms = now;
    if (dt > 0.1f) dt = 0.1f;

    game.game_time += dt;

    // simple periodic enemy spawn (if game.cpp doesn't already do waves)
    static float spawn_timer = 0.0f;
    spawn_timer += dt;
    if (spawn_timer > 3.0f && game.enemy_count < MAX_ENEMIES) {
        game_spawn_enemy(&game, ENEMY_SCOUT);
        spawn_timer = 0.0f;
    }

    game_update(&game, dt);
}

// -----------------------------------------------------------------------------
// Render game into framebuffer, then to LED matrix + OLED
// -----------------------------------------------------------------------------

static void render_game_to_framebuffer() {
    // 1. Draw map (background + path with textures)
    map_render_draw(&game);
    
    // 2. Draw decorations (trees, rocks, lakes) - BEFORE game objects
    map_render_decorations();

    // 3. Draw game objects (tower slots, enemies, towers, projectiles)
    game_draw(&game);

    // 4. If in placement mode, show range indicator at current slot
    if (show_placement_mode && game.tower_slot_count > 0) {
        TowerSlot* slot = &game.tower_slots[current_slot_index];
        
        // Get range for the selected tower type
        const TowerStats* stats = &TOWER_STATS_TABLE[game.selected_tower];
        draw_tower_range(slot->x, slot->y, stats->range);
        
        // Highlight current tower slot cursor (on top of range)
        // Make it pulse by varying brightness based on game time
        int brightness = 128 + (int)(127.0f * sinf(game.game_time * 4.0f));
        
        int cx = slot->x;
        int cy = slot->y;
        
        // Draw a 5x5 highlight box with pulsing color
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                // Only draw border, not filled
                if (abs(dx) == 2 || abs(dy) == 2) {
                    int px = cx + dx;
                    int py = cy + dy;
                    if (px >= 0 && px < MATRIX_WIDTH &&
                        py >= 0 && py < MATRIX_HEIGHT) {
                        set_pixel(px, py, Color(brightness, brightness, 255));
                    }
                }
            }
        }
    }
}

static void render_oled_ui() {
    char line1[17];
    char line2[17];

    if (show_placement_mode) {
        // Show tower selection info when in placement mode
        const char* tower_names[] = {"MG", "Cannon", "Sniper", "Radar"};
        const TowerStats* stats = &TOWER_STATS_TABLE[game.selected_tower];
        
        snprintf(line1, sizeof(line1), "%s $%d R%.0f",
                 tower_names[game.selected_tower],
                 stats->cost,
                 stats->range);
        snprintf(line2, sizeof(line2), "Money:%4d", game.money);
    } else {
        // Normal game info
        snprintf(line1, sizeof(line1), "Money:%4d", game.money);
        snprintf(line2, sizeof(line2), "Lives:%3d", game.lives);
    }

    oled_print(line1, line2);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
void render_matrix() {
    for (;;) {
        render_frame();
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking();
            swap_frames();
        }
    }
}


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
        // Check RFID and joystick using timer-based flags (no more sample_peripherals)
        check_tower_selection();  // Only runs when rfid_flag is set (1 second timer)
        handle_joystick();        // Only runs when joystick_flag is set (25ms timer)
        
        update_game();
        render_game_to_framebuffer();
        render_oled_ui();
        multicore_fifo_push_blocking(1);

        sleep_ms(40);  // ~25 FPS
    }

    return 0;
}