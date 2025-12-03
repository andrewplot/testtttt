#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "rfid_reader_uart.hh"
#include "pn532_uart.hh"

static pn532_uart_t pn532;
static bool pn532_ready = false;

void pn532_uart_reader_init(void) {
    // RP2350 Proton Board connections for UART
    const uint TX_PIN = 0;  // GPIO 32 = UART0 TX -> PN532 RX
    const uint RX_PIN = 1;  // GPIO 33 = UART0 RX <- PN532 TX
    const uint BAUD_RATE = 115200;
        
    // Initialize UART interface
    pn532_uart_init(&pn532, uart0, TX_PIN, RX_PIN, BAUD_RATE);
    
    // Give PN532 time to boot
    sleep_ms(500);
    
    // Try to get firmware version
    uint32_t ver = pn532_uart_get_firmware_version(&pn532);
    
    if (ver == 0) {
        pn532_ready = false;
        return;
    }
    
    // Extract version info
    uint8_t ic = (ver >> 24) & 0xFF;
    uint8_t ver_major = (ver >> 16) & 0xFF;
    uint8_t ver_minor = (ver >> 8) & 0xFF;
    uint8_t support = ver & 0xFF;
    
    if (ic != 0x32) {
    }
    
    // Configure SAM
    if (!pn532_uart_sam_config(&pn532)) {
        pn532_ready = false;
        return;
    }
    
    pn532_ready = true;
}

bool pn532_uart_read_uid(uint8_t *uid, uint8_t *uid_len) {
    if (!pn532_ready) {
        static bool warned = false;
        if (!warned) {
            warned = true;
        }
        return false;
    }
    
    // Use a timeout appropriate for tag reading
    return pn532_uart_read_passive_target(&pn532, uid, uid_len, 50);
}