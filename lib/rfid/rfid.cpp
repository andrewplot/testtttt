#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

#include "rfid.hh"
#include "rfid_reader_uart.hh"
#include "buzzer_pwm.hh"

#define RFID_TIMER_MS 1000

volatile bool rfid_flag = false;
uint8_t uid[10];
uint8_t uid_len;

TowerType match_monkey(uint8_t rfid_tag[10]) {
    switch (rfid_tag[1]) {
        case 0xC7: return MACHINE_GUN;
        case 0x76: return CANNON;
        case 0x35: return SNIPER;
        case 0xD7: return RADAR;
    }
}

void rfid_isr() {
    hw_clear_bits(&timer0_hw->intr, 1 << 1);

    rfid_flag = true;

    uint32_t target = timer0_hw->timerawl + RFID_TIMER_MS * 1000;
    timer0_hw->alarm[1] = target;
}

void init_rfid() {
    pn532_uart_reader_init();
    
    timer0_hw->inte |= 1 << 1;

    uint alarm1_irq = timer_hardware_alarm_get_irq_num(timer0_hw, 1);
    irq_set_exclusive_handler(alarm1_irq, rfid_isr);
    irq_set_enabled(alarm1_irq, true);

    uint32_t target = timer0_hw->timerawl + RFID_TIMER_MS * 1000;
    timer0_hw->alarm[1] = target;
}

TowerType sample_rfid() {
    if (pn532_uart_read_uid(uid, &uid_len)) {
        printf("Tag scanned\n");
        victory_sound();
        return match_monkey(uid);
    } else {
        printf("No tag\n");
        return BLANK;
    }
}

