#ifndef MATRIX_H
#define MATRIX_H

#include "color.hh"

#define MATRIX_ROWS 32
#define MATRIX_COLS 64

// External access to framebuffers for optimization
extern Color frames[2][MATRIX_ROWS][MATRIX_COLS];
extern int frame_index;

/*  NOTES:

    Row select; DCBA (meaning D = 8, C = 4, etc.)
        - This means 1011 = Row 11, 1001 = Row 9, etc 

*/

/**
 * @brief initializes matrix pins and buffer to have grass
 */
void init_matrix();

/**
 * @brief swaps framebuffers for double buffering
 */
void swap_frames();

/**
 * @brief renders one frame
 */
void render_frame();

/**
 * @brief adds predefined path to framebuffer
 * 
 * @param towers pointer to first element of Tower array
 */
void set_path();

/**
 * @brief adds tree to framebuffer at pos (x, y)
 * 
 * @param x top left x value of tree
 * @param y top left y value of tree
 */
void set_tree(int x, int y);

/**
 * @brief set pixel to 'color' at pos (x, y)
 * 
 * @param x x value
 * @param y y value
 * @param color color of pixel
 */
void set_pixel(int x, int y, Color color);

#endif // MATRIX_H