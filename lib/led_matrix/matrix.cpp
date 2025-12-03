#include "matrix.hh"
#include <math.h>
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "sprites.hh"
#include "../pins/pin-definitions.hh"


#define MATRIX_ROWS 32
#define MATRIX_COLS 64
#define GAMMA 2.9

Color frames[2][MATRIX_ROWS][MATRIX_COLS];
int frame_index = 0;

static uint8_t gamma_lut[256];

static inline void my_gpio_put(uint pin, bool val) {
    if (val) sio_hw->gpio_set = 1u << pin;
    else sio_hw->gpio_clr = 1u << pin;
}

static inline void set_row_pins(int row) {
    uint32_t pin_mask = (1u << A) | (1u << B) | (1u << C) | (1u << D);
    uint32_t pin_values = (((row >> 0) & 1) << A) |
                          (((row >> 1) & 1) << B) |
                          (((row >> 2) & 1) << C) |
                          (((row >> 3) & 1) << D);

    sio_hw->gpio_out = (sio_hw->gpio_out & ~pin_mask) | pin_values;
}

static inline void pulse_pin(int pin, int loops) {
    sio_hw->gpio_set = (1u << pin);
    for (volatile int i = 0; i < loops; ++i);
    sio_hw->gpio_clr = (1u << pin);
}

void init_matrix_pins() {
    // gpio_init(25);
    // gpio_set_dir(25, true);

    for (int pin = 5; pin < 20; pin++) {
        if (pin == 8) continue;

        gpio_init(pin);
        gpio_set_dir (pin, GPIO_OUT);
        gpio_set_slew_rate(pin, GPIO_SLEW_RATE_FAST);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_8MA);
    }
    sio_hw->gpio_clr = 0xFFFE0;
}

void init_framebuffers(Color color) {
    for (int row = 0; row < MATRIX_ROWS; row++) {
        for (int col = 0; col < MATRIX_COLS; col++) {
            frames[0][row][col] = color;
            frames[1][row][col] = color;

        }
    }
}

void init_gamma_lut() {
    for (int i = 0; i < 256; i++) {
        gamma_lut[i] = (uint8_t)(pow(i / 255.0, GAMMA) * 255.0);
    }
}


void init_matrix() {
    init_matrix_pins();
    init_framebuffers(GRASS);
    init_gamma_lut();
}

void swap_frames() {
    frame_index = !frame_index;
}

void reset_row_sel() {
    for (int row_sel = A; row_sel <= D; row_sel++) {
        my_gpio_put(row_sel, 0);
    }
}

void set_rgb_pins(int row, int col, int plane) {
    Color top = frames[!frame_index][row][col];
    Color bottom = frames[!frame_index][row+16][col];

    uint8_t top_r = gamma_lut[top.r];
    uint8_t top_g = gamma_lut[top.g];
    uint8_t top_b = gamma_lut[top.b];

    uint8_t bottom_r = gamma_lut[bottom.r];
    uint8_t bottom_g = gamma_lut[bottom.g];
    uint8_t bottom_b = gamma_lut[bottom.b];

    my_gpio_put(R1, ((top_r >> plane) & 0x1));
    my_gpio_put(G1, ((top_g >> plane) & 0x1));
    my_gpio_put(B1, ((top_b >> plane) & 0x1));

    my_gpio_put(R2, ((bottom_r >> plane) & 0x1));
    my_gpio_put(G2, ((bottom_g >> plane) & 0x1));
    my_gpio_put(B2, ((bottom_b >> plane) & 0x1));
}

void render_frame() {
    reset_row_sel();

    for (int plane = 5; plane >= 0; plane--) {
        for (int row = 0; row <  MATRIX_ROWS / 2; row++) {
            sio_hw->gpio_set = (1u << OE);
            sio_hw->gpio_set = (1u << 25);
            
            set_row_pins(row);
            
            for (int col = 0; col < MATRIX_COLS; col++) {
                set_rgb_pins(row, col, plane);
                pulse_pin(CLK, 3);
            }

            pulse_pin(LAT, 3);
            sio_hw->gpio_clr = (1u << OE);
            sio_hw->gpio_clr = (1u << 25);
            
            sleep_us(12 * (1 << plane));
        }
    }
}

void set_path() {
    for (int col = 0; col < 18; col++) {
        frames[frame_index][14][col] = PATH;
        frames[frame_index][15][col] = PATH;
        frames[frame_index][16][col] = PATH;
    }

    for (int row = 5; row < 17; row++) {
        frames[frame_index][row][16] = PATH;
        frames[frame_index][row][17] = PATH;
        frames[frame_index][row][18] = PATH;
    }
    
    for (int col = 16; col < 33; col++) {
        frames[frame_index][5][col] = PATH;
        frames[frame_index][6][col] = PATH;
        frames[frame_index][7][col] = PATH;

    }

    for (int row = 5; row < 27; row++) {
        frames[frame_index][row][30] = PATH;
        frames[frame_index][row][31] = PATH;
        frames[frame_index][row][32] = PATH;
    }

    for (int col = 33; col < 49; col++) {
        frames[frame_index][24][col] = PATH;
        frames[frame_index][25][col] = PATH;
        frames[frame_index][26][col] = PATH;
    }
    
    for (int row = 14; row < 27; row++) {
        frames[frame_index][row][46] = PATH;
        frames[frame_index][row][47] = PATH;
        frames[frame_index][row][48] = PATH;
    }

    for (int col = 46; col < 64; col++) {
        frames[frame_index][14][col] = PATH;
        frames[frame_index][15][col] = PATH;
        frames[frame_index][16][col] = PATH;
    }
}

void set_tree(int x, int y) {
    const Color* sprite = get_sprite_tree();

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            frames[frame_index][row + y][col + x] = sprite[row * 3 + col];
        }
    }
}

void set_pixel(int x, int y, Color color) {
    frames[frame_index][y][x] = color;
}