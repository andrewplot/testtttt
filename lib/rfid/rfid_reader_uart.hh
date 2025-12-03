#ifndef RFID_READER_UART_HH
#define RFID_READER_UART_HH

#include <stdint.h>
#include <stdbool.h>

/**
 * Initialize PN532 via UART
 * Sets up UART0 on GPIO 0 (TX) and GPIO 1 (RX) at 115200 baud
 * Configures PN532 and performs SAMConfig
 */
void pn532_uart_reader_init(void);

/**
 * Try to read a tag UID
 * 
 * @param uid Buffer to store UID (at least 10 bytes)
 * @param uid_len Pointer to store UID length
 * @return true if tag detected and UID read, false otherwise
 */
bool pn532_uart_read_uid(uint8_t *uid, uint8_t *uid_len);

#endif // RFID_READER_UART_H