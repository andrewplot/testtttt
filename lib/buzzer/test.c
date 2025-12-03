#include <stdio.h>
#include "pico/stdlib.h"
#include "buzzer_pwm.hh"

// Buzzer GPIO pin configuration
#define BUZZER_PIN 15  // Change this to whatever pin your buzzer is connected to

int main() {
    // Initialize USB serial
    stdio_init_all();
    
    // Wait for USB serial connection
    sleep_ms(2000);
    
    printf("\r\n");
    printf("========================================\r\n");
    printf("    PWM Buzzer Control Test Program    \r\n");
    printf("           RP2350 Proton Board          \r\n");
    printf("========================================\r\n");
    printf("\r\n");
    
    // Initialize PWM buzzer
    printf("Initializing PWM buzzer on GPIO %d...\r\n", BUZZER_PIN);
    buzzer_pwm_init(BUZZER_PIN);
    printf("PWM Buzzer initialized!\r\n\r\n");
    
    // Instructions
    printf("Commands:\r\n");
    printf("Frequency Beeps:\r\n");
    printf("  1 - Low beep (500 Hz)\r\n");
    printf("  2 - Medium beep (1000 Hz)\r\n");
    printf("  3 - High beep (2000 Hz)\r\n");
    printf("  4 - Alarm sound (2500 Hz)\r\n");
    printf("\r\n");
    printf("Musical Notes:\r\n");
    printf("  q - C4 (262 Hz)\r\n");
    printf("  w - D4 (294 Hz)\r\n");
    printf("  e - E4 (330 Hz)\r\n");
    printf("  r - F4 (349 Hz)\r\n");
    printf("  t - G4 (392 Hz)\r\n");
    printf("  y - A4 (440 Hz)\r\n");
    printf("  u - B4 (494 Hz)\r\n");
    printf("  i - C5 (523 Hz)\r\n");
    printf("\r\n");
    printf("Melodies:\r\n");
    printf("  m - Play simple melody\r\n");
    printf("  s - Startup sound\r\n");
    printf("  a - Success sound\r\n");
    printf("  x - Error sound\r\n");
    printf("\r\n");
    printf("Volume:\r\n");
    printf("  + - Increase volume\r\n");
    printf("  - - Decrease volume\r\n");
    printf("\r\n");
    printf("Control:\r\n");
    printf("  o - Turn ON continuous tone (1kHz)\r\n");
    printf("  f - Turn OFF (stop)\r\n");
    printf("  h - Show help\r\n");
    printf("========================================\r\n\r\n");
    
    printf("Ready! Press a key...\r\n");
    
    uint8_t volume = 50;  // Current volume level
    
    while (1) {
        // Check if a character is available
        int c = getchar_timeout_us(0);  // Non-blocking read
        
        if (c != PICO_ERROR_TIMEOUT) {
            char input = (char)c;
            
            switch (input) {
                // Frequency beeps
                case '1':
                    printf("Low beep (500 Hz)\r\n");
                    buzzer_beep(FREQ_LOW, 200);
                    break;
                    
                case '2':
                    printf("Medium beep (1000 Hz)\r\n");
                    buzzer_beep(FREQ_MEDIUM, 200);
                    break;
                    
                case '3':
                    printf("High beep (2000 Hz)\r\n");
                    buzzer_beep(FREQ_HIGH, 200);
                    break;
                    
                case '4':
                    printf("Alarm sound (2500 Hz)\r\n");
                    buzzer_beep(FREQ_ALARM, 500);
                    break;
                
                // Musical notes
                case 'q':
                case 'Q':
                    printf("C4 (262 Hz)\r\n");
                    buzzer_play_note(NOTE_C4, 300);
                    break;
                    
                case 'w':
                case 'W':
                    printf("D4 (294 Hz)\r\n");
                    buzzer_play_note(NOTE_D4, 300);
                    break;
                    
                case 'e':
                case 'E':
                    printf("E4 (330 Hz)\r\n");
                    buzzer_play_note(NOTE_E4, 300);
                    break;
                    
                case 'r':
                case 'R':
                    printf("F4 (349 Hz)\r\n");
                    buzzer_play_note(NOTE_F4, 300);
                    break;
                    
                case 't':
                case 'T':
                    printf("G4 (392 Hz)\r\n");
                    buzzer_play_note(NOTE_G4, 300);
                    break;
                    
                case 'y':
                case 'Y':
                    printf("A4 (440 Hz)\r\n");
                    buzzer_play_note(NOTE_A4, 300);
                    break;
                    
                case 'u':
                case 'U':
                    printf("B4 (494 Hz)\r\n");
                    buzzer_play_note(NOTE_B4, 300);
                    break;
                    
                case 'i':
                case 'I':
                    printf("C5 (523 Hz)\r\n");
                    buzzer_play_note(NOTE_C5, 300);
                    break;
                
                // Melodies
                case 'm':
                case 'M':
                    printf("Playing melody...\r\n");
                    {
                        const uint32_t melody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
                        const uint32_t durations[] = {200, 200, 200, 400};
                        buzzer_play_melody(melody, durations, 4);
                    }
                    printf("Done!\r\n");
                    break;
                    
                case 's':
                case 'S':
                    printf("Startup sound!\r\n");
                    {
                        const uint32_t startup[] = {FREQ_LOW, FREQ_MEDIUM, FREQ_HIGH};
                        const uint32_t durations[] = {100, 100, 200};
                        buzzer_play_melody(startup, durations, 3);
                    }
                    break;
                    
                case 'a':
                case 'A':
                    printf("Success sound!\r\n");
                    {
                        const uint32_t success[] = {NOTE_C5, NOTE_E5, NOTE_G5};
                        const uint32_t durations[] = {100, 100, 300};
                        buzzer_play_melody(success, durations, 3);
                    }
                    break;
                    
                case 'x':
                case 'X':
                    printf("Error sound!\r\n");
                    {
                        const uint32_t error[] = {FREQ_HIGH, 0, FREQ_HIGH};
                        const uint32_t durations[] = {100, 50, 100};
                        buzzer_play_melody(error, durations, 3);
                    }
                    break;
                
                // Volume control
                case '+':
                case '=':
                    volume += 10;
                    if (volume > 100) volume = 100;
                    buzzer_set_volume(volume);
                    printf("Volume: %d%%\r\n", volume);
                    buzzer_beep(FREQ_MEDIUM, 100);
                    break;
                    
                case '-':
                case '_':
                    if (volume >= 10) volume -= 10;
                    buzzer_set_volume(volume);
                    printf("Volume: %d%%\r\n", volume);
                    buzzer_beep(FREQ_MEDIUM, 100);
                    break;
                
                // Control
                case 'o':
                case 'O':
                    printf("Buzzer ON (1000 Hz continuous, press 'f' to stop)\r\n");
                    buzzer_play_tone(FREQ_MEDIUM, 0);  // 0 = continuous
                    break;
                    
                case 'f':
                case 'F':
                    printf("Buzzer OFF\r\n");
                    buzzer_stop();
                    break;
                    
                case 'h':
                case 'H':
                    printf("\r\nCommands:\r\n");
                    printf("  1-4: Different frequency beeps\r\n");
                    printf("  q-i: Musical notes (C4-C5)\r\n");
                    printf("  m: Play melody\r\n");
                    printf("  s/a/x: Sound effects\r\n");
                    printf("  +/-: Volume control\r\n");
                    printf("  o: ON, f: OFF\r\n\r\n");
                    break;
                    
                case '\r':
                case '\n':
                    // Ignore newlines
                    break;
                    
                default:
                    printf("Unknown: '%c' (press 'h' for help)\r\n", input);
                    break;
            }
        }
        
        // Small delay
        sleep_ms(10);
    }
    
    return 0;
}