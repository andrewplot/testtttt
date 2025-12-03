// main.cpp – RP2350 + PicoSDK + PlatformIO tower defense game

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "game_types.h"
#include "map_render.hh"
#include "joystick.hh"
#include "oled_display.hh"
#include "buzzer_pwm.hh"
#include "rfid_bridge.hh"
#include "pin-definitions.hh"


/*
TO IMPLEMENT:
-abilities
-banner plane
-map file w decorations
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
// Joystick → choose tower slot index
// -----------------------------------------------------------------------------

static void handle_joystick() {
    JoystickDirection jx = sample_js_x();
    bool sel = sample_js_select();

    printf("Joystick: jx=%d, sel=%d, placement_mode=%d\n", jx, sel, show_placement_mode);

    // Only navigate if in placement mode
    if (show_placement_mode && game.tower_slot_count > 0) {
        if (jx == right) {
            current_slot_index++;
            if (current_slot_index >= game.tower_slot_count)
                current_slot_index = 0;
            printf("Moved to slot %d\n", current_slot_index);
        } else if (jx == left) {
            current_slot_index--;
            if (current_slot_index < 0)
                current_slot_index = game.tower_slot_count - 1;
            printf("Moved to slot %d\n", current_slot_index);
        }
    }

    // On button press, try place tower at current slot
    static bool last_sel = false;
    if (sel && !last_sel && show_placement_mode && game.tower_slot_count > 0) {
        printf("Attempting to place tower...\n");
        TowerSlot* slot = &game.tower_slots[current_slot_index];

        if (!slot->occupied) {
            bool ok = game_place_tower(&game,
                                       game.selected_tower,
                                       slot->x,
                                       slot->y);
            if (ok) {
                slot->occupied = true;
                beep_ok();
                show_placement_mode = false;  // Exit placement mode after placing
                printf("Tower placed successfully!\n");
            } else {
                error_sound();
                printf("Tower placement failed (not enough money?)\n");
            }
        } else {
            error_sound();
            printf("Slot already occupied!\n");
        }
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
        
        // Debug output
        static int debug_counter = 0;
        if (debug_counter++ % 30 == 0) {  // Print every 30 frames
            printf("PLACEMENT MODE ACTIVE - Slot %d at (%d, %d)\n", 
                   current_slot_index, slot->x, slot->y);
        }
        
        // Get range for the selected tower type
        const TowerStats* stats = &TOWER_STATS_TABLE[game.selected_tower];
        draw_tower_range(slot->x, slot->y, stats->range);
        
        // Highlight current tower slot cursor (on top of range)
        int cx = slot->x;
        int cy = slot->y;
        // simple 3x3 highlight border in bright white
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int px = cx + dx;
                int py = cy + dy;
                if (px >= 0 && px < MATRIX_WIDTH &&
                    py >= 0 && py < MATRIX_HEIGHT) {
                    set_pixel(px, py, Color(255, 255, 255));
                }
            }
        }
    }
}

static void render_oled_ui() {
    char line1[17];
    char line2[17];

    snprintf(line1, sizeof(line1), "Money:%4d", game.money);
    snprintf(line2, sizeof(line2), "Lives:%3d", game.lives);

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

    while (true) {
        sample_peripherals();
        handle_joystick();
        update_game();
        render_game_to_framebuffer();
        render_oled_ui();
        multicore_fifo_push_blocking(1);

        sleep_ms(40);  // ~__ FPS
    }

    return 0;
}