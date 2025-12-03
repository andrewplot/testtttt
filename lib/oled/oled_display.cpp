#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "oled_display.hh"
#include "../pins/pin-definitions.hh"

static const uint8_t heart[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

static const uint8_t dollar[8] = {
    0b00100,
    0b11111,
    0b10100,
    0b11111,
    0b00101,
    0b11111,
    0b00100,
    0b00000
};

void send_spi_cmd(spi_inst_t* spi, int value) {
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = value;
}

void send_spi_data(spi_inst_t* spi, int value) {
    int data_value = 0x200 | value;
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    spi_get_hw(spi)->dr = data_value;
}


void oled_write_char(uint8_t row, uint8_t col, uint8_t ch) {
    uint8_t addr = (row == 0 ? 0x80 : 0xC0) + col;
    send_spi_cmd(spi1, addr);
    send_spi_data(spi1, ch);
}

void oled_create_char(uint8_t location, const uint8_t *pattern) {
    location &= 0x07; // valid: 0–7
    send_spi_cmd(spi1, 0x40 | (location << 3)); // set CGRAM address
    for (int i = 0; i < 8; i++) {
        send_spi_data(spi1, pattern[i]);
    }
}

void init_oled_pins() {
    spi_init(spi1, 10000);
    spi_set_format(spi1, 10, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    gpio_set_function(OLED_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(OLED_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(OLED_SPI_CSn, GPIO_FUNC_SPI);
}

void init_oled() {
    init_oled_pins();
    
    sleep_ms(1);
    send_spi_cmd(spi1, 0x38);
    send_spi_cmd(spi1, 0x0C);
    send_spi_cmd(spi1, 0x01);
    sleep_ms(2);
    send_spi_cmd(spi1, 0x06);

    oled_create_char(1, heart); // heart becomes CGRAM 1
    oled_create_char(2, dollar);
}

void cd_write_line(int row, const char *s) {
    int addr_cmd = (row == 0) ? 0x80 : 0xC0;
    send_spi_cmd(spi1, addr_cmd);
    sleep_ms(40);

    for (int i = 0; i < 16; i++) {
        char c = (s && s[i]) ? s[i] : ' '; // checks if character is valid or a null (space)
        send_spi_data(spi1, (int)c);
    }
}

void oled_print(const char *str1, const char *str2) {
    char buf1[17];
    char buf2[17];

    if (!str1) str1 = "";
    if (!str2) str2 = "";

    size_t len1 = strlen(str1);
    if (len1 > 16) len1 = 16;
    memcpy(buf1, str1, len1);
    memset(buf1 + len1, ' ', 16 - len1);
    buf1[16] = '\0';

    size_t len2 = strlen(str2);
    if (len2 > 16) len2 = 16;
    memcpy(buf2, str2, len2);
    memset(buf2 + len2, ' ', 16 - len2);
    buf2[16] = '\0';

    cd_write_line(0, buf1);
    cd_write_line(1, buf2);
}