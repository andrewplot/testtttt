// main.cpp – RP2350 + PicoSDK + PlatformIO tower defense game

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "game_types.h"
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
void set_path();
void set_tree(int x, int y);
void set_pixel(int x, int y, Color color);

// -----------------------------------------------------------------------------
// Global game data
// -----------------------------------------------------------------------------

GameState game;
Color framebuffer[MATRIX_HEIGHT][MATRIX_WIDTH];

uint32_t last_time_ms = 0;

// Cursor over tower slots
int current_slot_index = 0;

// -----------------------------------------------------------------------------
// Helper: simple buzzer beep
// -----------------------------------------------------------------------------

// static void beep_ok() {
//     buzzer_play_tone(NOTE_C5, 80);
// }

// static void beep_error() {
//     buzzer_play_tone(NOTE_C4, 40);
//     sleep_ms(30);
//     buzzer_play_tone(NOTE_C4, 120);
// }

// -----------------------------------------------------------------------------
// Initialize everything
// -----------------------------------------------------------------------------

static void setup_hardware() {
    stdio_init_all();

    // Matrix, joystick, OLED, RFID, buzzer
    init_matrix();
    set_path();          // draw static path once
    init_joystick();
    rfid_setup();
    init_oled();


    // buzzer on whatever pin your board uses (from pin-definitions.hh)
    buzzer_pwm_init();
    buzzer_set_volume(40);  // 40% duty

    // Initialize game
    game_init(&game);
    game.selected_tower = TOWER_MACHINE_GUN;

    // Spawn one test enemy so you see something move
    game_spawn_enemy(&game, ENEMY_SCOUT);

    last_time_ms = to_ms_since_boot(get_absolute_time());
}

// -----------------------------------------------------------------------------
// Joystick → choose tower slot index
// -----------------------------------------------------------------------------

static void handle_joystick() {
    JoystickDirection jx = sample_js_x();
    JoystickDirection jy = sample_js_y();
    bool sel = sample_js_select();

    // Horizontal navigation over tower slots
    if (game.tower_slot_count > 0) {
        if (jx == right) {
            current_slot_index++;
            if (current_slot_index >= game.tower_slot_count)
                current_slot_index = 0;
        } else if (jx == left) {
            current_slot_index--;
            if (current_slot_index < 0)
                current_slot_index = game.tower_slot_count - 1;
        }
    }

    // On button press, try place tower at current slot
    static bool last_sel = false;
    if (sel && !last_sel && game.tower_slot_count > 0) {
        TowerSlot* slot = &game.tower_slots[current_slot_index];

        if (!slot->occupied) {
            bool ok = game_place_tower(&game,
                                       game.selected_tower,
                                       slot->x,
                                       slot->y);
            if (ok) {
                slot->occupied = true;
                // beep_ok();
            } else {
                // beep_error();
            }
        } else {
            // beep_error();
        }
    }
    last_sel = sel;
}

// -----------------------------------------------------------------------------
// RFID → set selected tower type
// -----------------------------------------------------------------------------

// void handle_rfid(GameState* game) {
//     int code = rfid_get_tower_code();  // 0..4

//     switch (code) {
//         case 1: game->selected_tower = TOWER_MACHINE_GUN; break;
//         case 2: game->selected_tower = TOWER_CANNON;      break;
//         case 3: game->selected_tower = TOWER_SNIPER;      break;
//         case 4: game->selected_tower = TOWER_RADAR;       break;
//         default: return; // nothing scanned
//     }

//     printf("Selected tower code %d\n", code);
//     beep_ok();
// }


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
    // Grass background
    for (int y = 0; y < MATRIX_HEIGHT; ++y) {
        for (int x = 0; x < MATRIX_WIDTH; ++x) {
            set_pixel(x, y, Color(0, 40, 0));
            //framebuffer[y][x] = Color(0, 40, 0);
        }
    }

    // Ask game.cpp to draw enemies, towers, projectiles into framebuffer
    game_draw(&game);

    // OPTIONAL: highlight current tower slot cursor
    if (game.tower_slot_count > 0) {
        TowerSlot* slot = &game.tower_slots[current_slot_index];
        int cx = slot->x;
        int cy = slot->y;
        // simple 3x3 highlight border
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

// static void push_framebuffer_to_matrix() {
//     // copy framebuffer to LED matrix driver’s frame buffer
//     for (int y = 0; y < MATRIX_HEIGHT; ++y) {
//         for (int x = 0; x < MATRIX_WIDTH; ++x) {
//             set_pixel(x, y, framebuffer[y][x]);
//         }
//     }
// }

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
        // handle_rfid(&game);
        handle_joystick();
        update_game();
        render_game_to_framebuffer();
        //push_framebuffer_to_matrix();
        render_oled_ui();
        multicore_fifo_push_blocking(1);

        sleep_ms(20);  // ~33 FPS
    }

    return 0;
}


/*
1. path
2. random background
3. decorations
4. tower sprites
5. wave system

*/