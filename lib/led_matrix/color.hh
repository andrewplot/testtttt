#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    constexpr Color() : r(0), g(0), b(0) {};
    constexpr Color(int _r, int _g, int _b) : r(_r), g(_g), b(_b){};
};

constexpr Color BLACK(  0,   0,   0);
constexpr Color WHITE(255, 255, 255);

constexpr Color GRASS( 48, 156,  48);
constexpr Color GRASS_DARK( 64, 128,  64);
constexpr Color PATH(255, 193,   7);
constexpr Color TREE_BROWN( 87,  62,   8);
constexpr Color TREE_GREEN( 75,  99,  42);

constexpr Color BLOON_RED(222,  36,  36);
constexpr Color BLOON_BLUE(  0,   0,   0);
// ... etc.

constexpr Color DART_RED(222,   8,   8);
constexpr Color DART_BROWN( 87,  62,  37);
constexpr Color DART_LIGHT(242, 174,  97);

constexpr Color NINJA_RED(222,   4,   4);
constexpr Color NINJA_WHITE(241, 242, 191);

constexpr Color BOMB_BLACK(  0,   0,   0);
constexpr Color BOMB_BROWN(242, 174,  97);

constexpr Color SNIPER_BROWN( 87,  62,  37);
constexpr Color SNIPER_LIGHT_BROWN(242, 174,  97);
constexpr Color SNIPER_DARK_GREEN( 75,  99,  42);
constexpr Color SNIPER_LIGHT_GREEN(145, 145,  63);

constexpr Color MONKEY_RED(222,   0,   0);
constexpr Color MONKEY_BROWN( 87,  62,  37);
constexpr Color MONKEY_LIGHT(242, 174,  97);

#endif // COLOR_H