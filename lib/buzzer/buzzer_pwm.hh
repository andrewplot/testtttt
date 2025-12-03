#ifndef BUZZER_PWM_H
#define BUZZER_PWM_H

#include <stdint.h>
#include <stdbool.h>

// Common musical note frequencies (in Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_G4S  415
#define NOTE_A4  440
#define NOTE_A4S  466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_G5S 831 
#define NOTE_A5  880
#define NOTE_A5S 932
#define NOTE_B5  988
#define NOTE_C6  1047

// Common beep frequencies
#define FREQ_LOW     500   // Low beep
#define FREQ_MEDIUM  1000  // Medium beep
#define FREQ_HIGH    2000  // High beep
#define FREQ_ALARM   2500  // Alarm sound

/**
 * Initialize PWM buzzer on a specific GPIO pin
 * 
 * @param pin GPIO pin number for the buzzer
 */
void buzzer_pwm_init();

/**
 * Play a tone at a specific frequency (non-blocking)
 * This function starts the tone immediately and returns.
 * Use buzzer_stop() to stop the tone, or use buzzer_beep() for timed tones.
 * 
 * @param frequency Frequency in Hz (e.g., 1000 = 1kHz)
 * @param duration_ms Ignored (kept for API compatibility, use buzzer_beep for timed tones)
 */
void buzzer_play_tone(uint32_t frequency, uint32_t duration_ms);

/**
 * Stop the buzzer (silence)
 */
void buzzer_stop(void);

/**
 * Play a beep at specified frequency (blocking)
 * This function will block for the duration of the beep.
 * 
 * @param frequency Frequency in Hz
 * @param duration_ms Duration of the beep (blocks during this time)
 */
void buzzer_beep(uint32_t frequency, uint32_t duration_ms);

void beep_ok();
/**
 * Play a musical note (blocking)
 * This function will block for the duration of the note.
 * 
 * @param note Note frequency (use NOTE_* defines)
 * @param duration_ms Duration in milliseconds (blocks during this time)
 */
void buzzer_play_note(uint32_t note, uint32_t duration_ms);

/**
 * Play a simple melody (blocking)
 * This function will block until the entire melody is complete.
 * 
 * @param frequencies Array of frequencies in Hz
 * @param durations Array of durations in ms
 * @param note_count Number of notes to play
 */
void buzzer_play_melody(const uint32_t *frequencies, const uint32_t *durations, unsigned int note_count);

/**
 * Set PWM duty cycle (volume control)
 * 
 * @param duty Duty cycle percentage (0-100)
 */
void buzzer_set_volume(uint8_t duty);

/**
 * Play sound effect 1 - Mario-style melody
 */
void victory_sound(void);

/**
 * Play sound effect 2 - Quick two-tone
 */
void damage_sound(void);

/**
 * Play sound effect 3 - Error sound
 */
void loss_sound(void);

void error_sound(void);

void start_sound(void);

#endif // BUZZER_PWM_H