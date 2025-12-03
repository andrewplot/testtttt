#include "buzzer_pwm.hh"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "../pins/pin-definitions.hh"

static unsigned int pwm_slice = 0;
static unsigned int pwm_channel = 0;
static bool buzzer_initialized = false;
static uint8_t current_volume = 50;  // Default 50% duty cycle
static uint32_t current_wrap = 0;    // Store wrap value

void buzzer_pwm_init() {
    // Configure GPIO for PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM); 
    
    // Get PWM slice and channel for this GPIO
    pwm_slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_channel = pwm_gpio_to_channel(BUZZER_PIN);
    
    // Set default configuration
    pwm_config config = pwm_get_default_config();
    pwm_init(pwm_slice, &config, false);  // Don't start yet
    
    // Set duty cycle to 50% (square wave)
    buzzer_set_volume(90);
    
    buzzer_initialized = true;
}

void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms) {
    if (!buzzer_initialized || frequency == 0) {
        buzzer_stop();
        return;
    }
    
    // Calculate PWM parameters
    // System clock is typically 125 MHz
    uint32_t clock_freq = clock_get_hz(clk_sys);
    
    // Calculate divider and wrap value for desired frequency
    // PWM frequency = clock_freq / (divider * wrap)
    // For good resolution, aim for wrap around 1000-10000
    
    uint32_t divider = 1;
    uint32_t wrap = clock_freq / frequency;
    
    // If wrap is too large, increase divider
    while (wrap > 65535 && divider < 255) {
        divider++;
        wrap = clock_freq / (frequency * divider);
    }
    
    // Limit wrap to valid range
    if (wrap > 65535) wrap = 65535;
    if (wrap < 2) wrap = 2;
    
    // Store wrap for later use
    current_wrap = wrap;
    
    // Configure PWM
    pwm_set_clkdiv(pwm_slice, (float)divider);
    pwm_set_wrap(pwm_slice, wrap - 1);
    
    // Set duty cycle based on current volume
    uint32_t level = (wrap * current_volume) / 100;
    pwm_set_chan_level(pwm_slice, pwm_channel, level);
    
    // Enable PWM
    pwm_set_enabled(pwm_slice, true);
}

void buzzer_stop(void) {
    if (!buzzer_initialized) return;
    
    // Disable PWM
    pwm_set_enabled(pwm_slice, false);
    
    // Set PWM level to 0 to ensure silence
    pwm_set_chan_level(pwm_slice, pwm_channel, 0);
}

void buzzer_beep(uint32_t frequency, uint32_t duration_ms) {
    buzzer_play_tone(frequency, 0);  // Start continuous tone
    if (duration_ms > 0) {
        sleep_ms(duration_ms);
        buzzer_stop();
    }
}

void beep_ok() {
    buzzer_play_tone(NOTE_C5, 80);
}

void buzzer_play_note(uint32_t note, uint32_t duration_ms) {
    buzzer_play_tone(note, 0);  // Start continuous tone
    if (duration_ms > 0) {
        sleep_ms(duration_ms);
        buzzer_stop();
    }
}

void buzzer_play_melody(const uint32_t *frequencies, const uint32_t *durations, unsigned int note_count) {
    if (!buzzer_initialized) return;
    
    for (unsigned int i = 0; i < note_count; i++) {
        if (frequencies[i] > 0) {
            buzzer_play_tone(frequencies[i], 0);  // Start tone
            sleep_ms(durations[i]);
            buzzer_stop();
        } else {
            // Frequency of 0 = rest/silence
            buzzer_stop();
            sleep_ms(durations[i]);
        }
        
        // Small gap between notes
        sleep_ms(20);
    }
}

void buzzer_set_volume(uint8_t duty) {
    if (duty > 100) duty = 100;
    current_volume = duty;
    
    // If PWM is currently running and we have a wrap value, update the level
    if (buzzer_initialized && current_wrap > 0) {
        uint32_t level = (current_wrap * current_volume) / 100;
        pwm_set_chan_level(pwm_slice, pwm_channel, level);
    }
}

void victory_sound(void) {
    // Victory
    const uint32_t melody[] = {NOTE_E5, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
    const uint32_t durations[] = {150, 150, 150, 150, 400};
    buzzer_play_melody(melody, durations, 5);
}

void damage_sound(void) {
    // Balloon Pop
    const uint32_t melody[] = {330, 250};
    const uint32_t durations[] = {70, 90};
    buzzer_play_melody(melody, durations, 2);
}

void error_sound(void) {
    // Error sound - double beep
    const uint32_t error[] = {FREQ_HIGH, 0, FREQ_HIGH};
    const uint32_t durations[] = {100, 50, 100};
    buzzer_play_melody(error, durations, 3);
}

void loss_sound(void){
    //Lose Sound
    const uint32_t lose[] = {NOTE_B5, NOTE_A5S, NOTE_G5S};
    const uint32_t durations[] = {300, 300, 600};
    buzzer_play_melody(lose, durations, 3);
}

void start_sound(void){
    //Wave Start
    const uint32_t error[] = {NOTE_D5, 0, NOTE_D5, 0, NOTE_D5, NOTE_G5, 0, NOTE_G5, 0, NOTE_G5};
    const uint32_t durations[] = {300, 50, 100, 30, 100, 500, 100, 50, 50, 50};
    buzzer_play_melody(error, durations, 10); 
}