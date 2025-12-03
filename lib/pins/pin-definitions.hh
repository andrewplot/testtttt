#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

/* DO NOT CHANGE!!! */
/* This is going to be finalized with the PCB */

// LED Matrix
#define E      5
#define B2     12
#define B1     10
#define R1     9
#define G1     7
#define R2     11
#define G2     6
#define A      13
#define C      14
#define CLK    15
#define OE     16
#define LAT    17
#define D      18
#define B      19

// SPI for OLED
#define OLED_SPI_CSn 25
#define OLED_SPI_SCK 26
#define OLED_SPI_TX  27

// I2C for RFID
#define RFID_TX      0
#define RFID_SDA     1

// Joystick
#define JOYSTICK_X   40
#define JOYSTICK_Y   41
#define JOYSTICK_SW  42


// House LED
#define LED_R        47
#define LED_G        46
#define LED_B        45

#define BUZZER_PIN 28


#endif