#include "pn532_uart.hh"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"

// PN532 Frame constants
#define PN532_PREAMBLE      0x00
#define PN532_STARTCODE1    0x00
#define PN532_STARTCODE2    0xFF
#define PN532_POSTAMBLE     0x00

#define PN532_HOST_TO_PN532 0xD4
#define PN532_PN532_TO_HOST 0xD5

// Commands
#define PN532_CMD_GETFIRMWAREVERSION  0x02
#define PN532_CMD_SAMCONFIGURATION    0x14
#define PN532_CMD_INLISTPASSIVETARGET 0x4A

// ACK frame
static const uint8_t PN532_ACK_FRAME[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

// Debug flag
#define DEBUG_PN532 0

// ====== Low-level UART helpers ======

static void uart_flush_rx(uart_inst_t *uart) {
    // Flush any pending data in UART RX buffer
    while (uart_is_readable(uart)) {
        uart_getc(uart);
    }
}

static bool uart_write_frame(pn532_uart_t *dev, const uint8_t *data, size_t len) {
#if DEBUG_PN532
    printf("UART Write: ");
    for (size_t i = 0; i < len && i < 20; i++) {
        printf("%02X ", data[i]);
    }
    if (len > 20) printf("...");
    printf("(%d bytes)\r\n", len);
#endif
    
    uart_write_blocking(dev->uart, data, len);
    return true;
}

// Wait for specific byte with timeout
static bool uart_wait_for_byte(uart_inst_t *uart, uint8_t expected, uint32_t timeout_ms) {
    absolute_time_t timeout_time = make_timeout_time_ms(timeout_ms);
    
    while (!time_reached(timeout_time)) {
        if (uart_is_readable_within_us(uart, 1000)) {  // 1ms polling
            uint8_t c = uart_getc(uart);
            if (c == expected) {
                return true;
            }
        }
    }
    return false;
}

// Read exact number of bytes with timeout
static bool uart_read_bytes(uart_inst_t *uart, uint8_t *buf, size_t len, uint32_t timeout_ms) {
    absolute_time_t timeout_time = make_timeout_time_ms(timeout_ms);
    size_t received = 0;
    
    while (received < len && !time_reached(timeout_time)) {
        if (uart_is_readable_within_us(uart, 1000)) {  // 1ms polling
            buf[received++] = uart_getc(uart);
        }
    }
    
    return (received == len);
}

// ====== PN532 Protocol Functions ======

static uint8_t calc_len_checksum(uint8_t len) {
    return (uint8_t)(~len + 1);
}

static uint8_t calc_data_checksum(uint8_t tfi, const uint8_t *data, uint8_t len) {
    uint16_t sum = tfi;
    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint8_t)(~sum + 1);
}

// Wake up PN532 from low power mode
static void pn532_wakeup(pn532_uart_t *dev) {
    // Flush any old data first
    uart_flush_rx(dev->uart);
    
    // Send wake-up sequence (55 00 00...)
    uint8_t wake[] = {0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uart_write_blocking(dev->uart, wake, sizeof(wake));
    sleep_ms(100);  // Give PN532 more time to wake up
    
    // Flush any echoed wake-up bytes
    uart_flush_rx(dev->uart);
}

// Wait for and verify ACK frame
static bool read_ack(pn532_uart_t *dev, uint32_t timeout_ms) {
    uint8_t buf[6];
    
    // Read all 6 bytes of the ACK frame at once
    // ACK format: 00 00 FF 00 FF 00
    if (!uart_read_bytes(dev->uart, buf, 6, timeout_ms)) {
#if DEBUG_PN532
        printf("read_ack: timeout reading ACK frame\r\n");
#endif
        return false;
    }
    
#if DEBUG_PN532
    printf("read_ack: ");
    for (int i = 0; i < 6; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\r\n");
#endif
    
    // Verify it matches the ACK pattern
    if (memcmp(buf, PN532_ACK_FRAME, 6) == 0) {
#if DEBUG_PN532
        printf("read_ack: ACK OK\r\n");
#endif
        return true;
    }
    
#if DEBUG_PN532
    printf("read_ack: ACK mismatch\r\n");
#endif
    return false;
}

// Send a command to PN532
static bool send_command(pn532_uart_t *dev, uint8_t cmd, const uint8_t *params, uint8_t params_len) {
    uint8_t len = params_len + 2;  // TFI + CMD + params
    uint8_t lcs = calc_len_checksum(len);
    uint8_t frame[8 + 255];
    size_t idx = 0;
    
    // Build frame: PRE + START1 + START2 + LEN + LCS + TFI + CMD + PARAMS + DCS + POST
    frame[idx++] = PN532_PREAMBLE;
    frame[idx++] = PN532_STARTCODE1;
    frame[idx++] = PN532_STARTCODE2;
    frame[idx++] = len;
    frame[idx++] = lcs;
    frame[idx++] = PN532_HOST_TO_PN532;
    frame[idx++] = cmd;
    
    if (params_len > 0 && params) {
        memcpy(frame + idx, params, params_len);
        idx += params_len;
    }
    
    // Calculate data checksum (TFI + CMD + params)
    // calc_data_checksum adds TFI to the sum, then sums the data array
    // So we pass CMD + params as the data array
    uint8_t dcs = calc_data_checksum(PN532_HOST_TO_PN532, &frame[6], len - 1);
    frame[idx++] = dcs;
    frame[idx++] = PN532_POSTAMBLE;
    
#if DEBUG_PN532
    printf("send_command: cmd=0x%02X, params_len=%d\r\n", cmd, params_len);
#endif
    
    return uart_write_frame(dev, frame, idx);
}

// Read response from PN532
static bool read_response(pn532_uart_t *dev,
                         uint8_t expected_cmd,
                         uint8_t *out, uint8_t out_len,
                         uint32_t timeout_ms) {
    uint8_t hdr[6];
    
    // Wait for preamble
    if (!uart_wait_for_byte(dev->uart, PN532_PREAMBLE, timeout_ms)) {
#if DEBUG_PN532
        printf("read_response: timeout waiting for preamble\r\n");
#endif
        return false;
    }
    
    // Small delay to let header bytes arrive
    sleep_ms(10);
    
    // Read header: START1 START2 LEN LCS TFI CMD
    if (!uart_read_bytes(dev->uart, hdr, 6, timeout_ms)) {
#if DEBUG_PN532
        printf("read_response: timeout reading header\r\n");
#endif
        return false;
    }
    
#if DEBUG_PN532
    printf("read_response: 00 ");
    for (int i = 0; i < 6; i++) {
        printf("%02X ", hdr[i]);
    }
    printf("\r\n");
#endif
    
    // Verify start codes
    if (hdr[0] != PN532_STARTCODE1 || hdr[1] != PN532_STARTCODE2) {
#if DEBUG_PN532
        printf("read_response: invalid start codes\r\n");
#endif
        return false;
    }
    
    // Verify length checksum
    uint8_t len = hdr[2];
    uint8_t lcs = hdr[3];
    if ((uint8_t)(len + lcs) != 0x00) {
#if DEBUG_PN532
        printf("read_response: LCS error (len=0x%02X, lcs=0x%02X)\r\n", len, lcs);
#endif
        return false;
    }
    
    // Verify TFI and response command
    uint8_t tfi = hdr[4];
    uint8_t rsp_cmd = hdr[5];
    
    if (tfi != PN532_PN532_TO_HOST) {
#if DEBUG_PN532
        printf("read_response: wrong TFI (0x%02X)\r\n", tfi);
#endif
        return false;
    }
    
    if (rsp_cmd != (expected_cmd + 1)) {
#if DEBUG_PN532
        printf("read_response: wrong cmd (0x%02X, expected 0x%02X)\r\n", 
               rsp_cmd, expected_cmd + 1);
#endif
        return false;
    }
    
    // Read payload + DCS + POSTAMBLE
    // len includes TFI + CMD, so payload_len = len - 2
    uint8_t payload_len = (len >= 2) ? (len - 2) : 0;
    uint8_t tail[255];
    
    if (!uart_read_bytes(dev->uart, tail, payload_len + 2, timeout_ms)) {
#if DEBUG_PN532
        printf("read_response: timeout reading payload\r\n");
#endif
        return false;
    }
    
    // Verify data checksum
    uint16_t sum = tfi + rsp_cmd;
    for (uint8_t i = 0; i < payload_len; i++) {
        sum += tail[i];
    }
    uint8_t dcs = tail[payload_len];
    
    if ((uint8_t)(sum + dcs) != 0x00) {
#if DEBUG_PN532
        printf("read_response: DCS error\r\n");
#endif
        return false;
    }
    
    // Verify postamble
    if (tail[payload_len + 1] != PN532_POSTAMBLE) {
#if DEBUG_PN532
        printf("read_response: missing postamble\r\n");
#endif
        return false;
    }
    
    // Copy payload to output buffer
    if (payload_len > out_len) payload_len = out_len;
    if (payload_len > 0 && out) {
        memcpy(out, tail, payload_len);
    }
    
#if DEBUG_PN532
    printf("read_response: success, payload_len=%d\r\n", payload_len);
#endif
    
    return true;
}

// ====== Public API ======

void pn532_uart_init(pn532_uart_t *dev, uart_inst_t *uart, uint tx_pin, uint rx_pin, uint baud_rate) {
    dev->uart = uart;
    
    // Initialize UART
    uart_init(uart, baud_rate);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    
    // Set UART format: 8N1
    uart_set_format(uart, 8, 1, UART_PARITY_NONE);
    
    // Enable UART FIFOs
    uart_set_fifo_enabled(uart, true);
    
}

uint32_t pn532_uart_get_firmware_version(pn532_uart_t *dev) {
#if DEBUG_PN532
    printf("pn532_uart_get_firmware_version: starting\r\n");
#endif
    
    // Wake up PN532
    pn532_wakeup(dev);
    
    // Send GetFirmwareVersion command
    if (!send_command(dev, PN532_CMD_GETFIRMWAREVERSION, NULL, 0)) {
#if DEBUG_PN532
        printf("get_firmware_version: send_command failed\r\n");
#endif
        return 0;
    }
    
    // Wait for ACK
    sleep_ms(50);
    
    if (!read_ack(dev, 1000)) {
#if DEBUG_PN532
        printf("get_firmware_version: read_ack failed\r\n");
#endif
        return 0;
    }
    
    // Read response
    uint8_t buf[8];
    if (!read_response(dev, PN532_CMD_GETFIRMWAREVERSION, buf, sizeof(buf), 1000)) {
#if DEBUG_PN532
        printf("get_firmware_version: read_response failed\r\n");
#endif
        return 0;
    }
    
    // buf[0..3] = IC, Ver, Rev, Support
    uint32_t version = ((uint32_t)buf[0] << 24) |
                       ((uint32_t)buf[1] << 16) |
                       ((uint32_t)buf[2] << 8)  |
                       ((uint32_t)buf[3]);
    
#if DEBUG_PN532
    printf("Firmware: IC=0x%02X, Ver=%d.%d, Support=0x%02X\r\n",
           buf[0], buf[1], buf[2], buf[3]);
#endif
    
    return version;
}

bool pn532_uart_sam_config(pn532_uart_t *dev) {
#if DEBUG_PN532
    printf("pn532_uart_sam_config: starting\r\n");
#endif
    
    // SAMConfiguration: Normal mode, timeout 0x14, use IRQ
    uint8_t params[3] = {0x01, 0x14, 0x01};
    
    if (!send_command(dev, PN532_CMD_SAMCONFIGURATION, params, 3)) {
#if DEBUG_PN532
        printf("sam_config: send_command failed\r\n");
#endif
        return false;
    }
    
    sleep_ms(50);
    
    if (!read_ack(dev, 1000)) {
#if DEBUG_PN532
        printf("sam_config: read_ack failed\r\n");
#endif
        return false;
    }
    
    uint8_t buf[4];
    if (!read_response(dev, PN532_CMD_SAMCONFIGURATION, buf, sizeof(buf), 1000)) {
#if DEBUG_PN532
        printf("sam_config: read_response failed\r\n");
#endif
        return false;
    }
    
#if DEBUG_PN532
    printf("sam_config: success\r\n");
#endif
    
    return true;
}

bool pn532_uart_read_passive_target(pn532_uart_t *dev,
                                    uint8_t *uid_buf,
                                    uint8_t *uid_len,
                                    uint32_t timeout_ms) {
    // InListPassiveTarget: max 1 target, 106 kbps Type A (0x00)
    uint8_t params[2] = {0x01, 0x00};
    
    // Flush RX buffer before sending command
    uart_flush_rx(dev->uart);
    
    if (!send_command(dev, PN532_CMD_INLISTPASSIVETARGET, params, 2)) {
        return false;
    }
    
    // Give PN532 more time - ACK should arrive within 50ms
    busy_wait_ms(50);
    
    if (!read_ack(dev, 1000)) {
        // If ACK fails, flush and return false
        uart_flush_rx(dev->uart);
        return false;
    }
    
    // Now wait for the actual response (this may timeout if no tag present)
    uint8_t buf[32];
    if (!read_response(dev, PN532_CMD_INLISTPASSIVETARGET, buf, sizeof(buf), timeout_ms)) {
        return false;  // No tag found or timeout
    }
    
    // Expected layout (for Type A):
    // buf[0] = NbTg (number of targets, should be 1)
    // buf[1] = Tg
    // buf[2..] = target data
    // buf[5] = UID length
    // buf[6..] = UID
    
    if (buf[0] < 1) {
        return false;  // No targets found
    }
    
    uint8_t length = buf[5];
    if (length == 0 || length > 10) {
        return false;  // Invalid UID length
    }
    
    if (uid_buf && uid_len) {
        *uid_len = length;
        for (uint8_t i = 0; i < length; i++) {
            uid_buf[i] = buf[6 + i];
        }
    }
    
    return true;
}