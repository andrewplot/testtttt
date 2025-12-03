#ifndef JOYSTICK_HH
#define JOYSTICK_HH

extern volatile bool joystick_flag;

enum JoystickDirection {
    left,
    right,
    up,
    down,
    center,
};

/**
 * @brief initialize joystick pins and adc
 */

void init_joystick(void);

/**
 * @brief return 1 if right, -1 if left, 0 if neither
 * 
 */
JoystickDirection sample_js_x(void);

/**
 * @brief return 1 if up, -1 if down, 0 if neither
 * 
 */
JoystickDirection sample_js_y(void);

/**
 * @brief return 1 if pressed, 0 if not 
 */
bool sample_js_select(void);

#endif // JOYSTICK_HH