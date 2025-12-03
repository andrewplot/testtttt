#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "rfid_reader_uart.hh"

int main() {
    stdio_init_all();
    
    sleep_ms(3000);
    
    printf("\r\n");
    printf("=====================================\r\n");
    printf("   PN532 NFC/RFID Reader Test (UART) \r\n");
    printf("         RP2350 Proton Board         \r\n");
    printf("=====================================\r\n");
    printf("\r\n");
    
    printf("Initializing PN532 over UART...\r\n");
    printf("UART Configuration:\r\n");
    printf(" - TX: GPIO 32 (to PN532 RX)\r\n");
    printf(" - RX: GPIO 33 (from PN532 TX)\r\n");
    printf(" - Baud: 115200\r\n");
    printf(" - Module switches: Both OFF\r\n");
    printf("\r\n");
    
    pn532_uart_reader_init();
    
    printf("\r\n");
    printf("Starting tag detection loop...\r\n");
    printf("Place an NFC/RFID tag near the reader.\r\n");
    printf("-------------------------------------\r\n");
    
    uint8_t uid[10];
    uint8_t uid_len;
    uint8_t last_uid[10];
    uint8_t last_uid_len = 0;
    bool tag_present = false;
    
    while (1) {
        if (pn532_uart_read_uid(uid, &uid_len)) {
            // ALWAYS print the UID every time we read it
            printf("TAG - UID: ");
            for (uint8_t i = 0; i < uid_len; i++) {
                if (i > 0) printf(":");
                printf("%02X", uid[i]);
            }
            printf(" (Length: %d bytes)\r\n", uid_len);
            
            tag_present = true;
        } else {
            if (tag_present) {
                printf("Tag removed\r\n");
                tag_present = false;
            }
        }
        
        sleep_ms(1000);  // Poll 4 times per second
    }
}