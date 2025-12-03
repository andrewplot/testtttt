#ifndef PN532_UART_HH
#define PN532_UART_HH

#include <stdint.h>
#include <stdbool.h>
#include "hardware/uart.h"

typedef struct {
    uart_inst_t *uart;
} pn532_uart_t;

/**
 * Initialize PN532 via UART
 * 
 * @param dev Pointer to pn532_uart_t structure
 * @param uart UART instance (uart0 or uart1)
 * @param tx_pin GPIO pin for UART TX
 * @param rx_pin GPIO pin for UART RX
 * @param baud_rate Baud rate (typically 115200)
 */
void pn532_uart_init(pn532_uart_t *dev, uart_inst_t *uart, uint tx_pin, uint rx_pin, uint baud_rate);

/**
 * Get firmware version from PN532
 * 
 * @param dev Pointer to pn532_uart_t structure
 * @return 32-bit firmware version (IC|Ver|Rev|Support) or 0 on failure
 */
uint32_t pn532_uart_get_firmware_version(pn532_uart_t *dev);

/**
 * Configure SAM (Secure Access Module)
 * 
 * @param dev Pointer to pn532_uart_t structure
 * @return true on success, false on failure
 */
bool pn532_uart_sam_config(pn532_uart_t *dev);

/**
 * Read passive ISO14443A target (MIFARE cards, etc.)
 * 
 * @param dev Pointer to pn532_uart_t structure
 * @param uid_buf Buffer to store UID (at least 10 bytes)
 * @param uid_len Pointer to store UID length
 * @param timeout_ms Timeout in milliseconds
 * @return true if tag detected, false otherwise
 */
bool pn532_uart_read_passive_target(pn532_uart_t *dev,
                                    uint8_t *uid_buf,
                                    uint8_t *uid_len,
                                    uint32_t timeout_ms);

#endif // PN532_UART_HH