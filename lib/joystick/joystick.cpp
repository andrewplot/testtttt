#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "../pins/pin-definitions.hh"

#include "joystick.hh"

#define ADC_MAX 4095
#define CENTER 2048
#define DEADZONE_PERCENT 50
#define JOYSTICK_TIMER_MS 25

volatile bool joystick_flag = false;

void joystick_isr() {
    hw_clear_bits(&timer0_hw->intr, 1 << 0);

    //sample js 
    joystick_flag = true;

    uint32_t target = timer0_hw->timerawl + JOYSTICK_TIMER_MS * 1000;
    timer0_hw->alarm[0] = target;
}

void init_joystick(void){
    adc_init();

    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(JOYSTICK_SW);
    gpio_set_dir(JOYSTICK_SW, GPIO_IN);
    gpio_pull_up(JOYSTICK_SW);

    timer0_hw->inte |= 1 << 0;

    uint alarm0_irq = timer_hardware_alarm_get_irq_num(timer0_hw, 0);
    irq_set_exclusive_handler(alarm0_irq, joystick_isr);
    irq_set_enabled(alarm0_irq, true);

    uint32_t target = timer0_hw->timerawl + JOYSTICK_TIMER_MS * 1000;
    timer0_hw->alarm[0] = target;    
}

JoystickDirection sample_js_x(void){
    adc_select_input(0);
    uint16_t value = adc_read();

    int deadzone = (ADC_MAX / 2) * DEADZONE_PERCENT / 100;
    if (value > CENTER + deadzone) return right;   // right
    else if (value < CENTER - deadzone) return left;  // left
    else return center;
}

JoystickDirection sample_js_y(void){
    adc_select_input(1);
    uint16_t value = adc_read();

    int deadzone = (ADC_MAX / 2) * DEADZONE_PERCENT / 100;
    if (value > CENTER + deadzone) return up;   // up
    else if (value < CENTER - deadzone) return down;  // down
    else return center;
}

bool sample_js_select(void) {
    return (gpio_get(JOYSTICK_SW) == 0); // LOW when pressed
}

