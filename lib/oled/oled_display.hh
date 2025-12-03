#include <stdint.h>
#include "hardware/spi.h"


/**
 * @brief initialized oled pins and special characters
 */
void init_oled();

/**
 * @brief prints message on both lines
 * 
 * @param lines 2 strings to be printed on the oled 
 */
void oled_print(const char str1[16], const char str2[16]);