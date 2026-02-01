#pragma once
#include <cstdint>

// Window Settings
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "Block Coding System";

// Color Structure
struct Color {
    uint8_t r, g, b, a;
};

// Predefined Colors
constexpr Color COLOR_WHITE = {255, 255, 255, 255};
constexpr Color COLOR_BLACK = {0, 0, 0, 255};
constexpr Color COLOR_BACKGROUND = {240, 240, 240, 255};
constexpr Color COLOR_BLOCK_BLUE = {100, 150, 255, 255};
constexpr Color COLOR_BLOCK_RED = {255, 100, 100, 255};
constexpr Color COLOR_BLOCK_GREEN = {100, 255, 100, 255};
