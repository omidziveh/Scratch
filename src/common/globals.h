#ifndef GLOBALS_H
#define GLOBALS_H

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BLOCK_CORNER_RADIUS = 8;

const int DEFAULT_TICK_RATE = 60;
const int DEFAULT_MAX_TICKS = 100000;

struct Color {
    int r, g, b, a;
};

const Color COLOR_WHITE = {255, 255, 255, 255};
const Color COLOR_BLACK = {0, 0, 0, 255};
const Color COLOR_RED = {255, 0, 0, 255};
const Color COLOR_GREEN = {0, 255, 0, 255};
const Color COLOR_BLUE = {0, 0, 255, 255};
const Color COLOR_GRAY = {128, 128, 128, 255};

const Color COLOR_MOTION = {74, 108, 212, 255};
const Color COLOR_CONTROL = {255, 171, 25, 255};
const Color COLOR_LOOKS = {148, 92, 213, 255};

#endif
