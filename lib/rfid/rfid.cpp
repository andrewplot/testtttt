#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

#include "rfid.hh"
#include "rfid_reader_uart.hh"
#include "buzzer_pwm.hh"

// Remove automatic timer - RFID only reads on demand now
uint8_t uid[10];
uint8_t uid_len;

HardwareTowerType match_monkey(uint8_t rfid_tag[10]) {
    switch (rfid_tag[1]) {
        case 0xC7: return MACHINE_GUN;
        case 0x76: return CANNON;
        case 0x35: return SNIPER;
        case 0xD7: return RADAR;
    }
    return BLANK;
}

void init_rfid() {
    pn532_uart_reader_init();
    printf("RFID initialized (on-demand mode)\n");
}

HardwareTowerType sample_rfid() {
    if (pn532_uart_read_uid(uid, &uid_len)) {
        printf("Tag scanned\n");
        victory_sound();
        return match_monkey(uid);
    } else {
        printf("No tag\n");
        return BLANK;
    }
}