#ifndef GLOBALS_H
#define GLOBALS_H


const int BLOCK_CORNER_RADIUS = 8;

// const int DEFAULT_TICK_RATE = 60;
// const int DEFAULT_MAX_TICKS = 100000;
const int DEFAULT_WATCHDOG_THRESHOLD = 10000;
const int LOOP_WATCHDOG_LIMIT = 1000;

// const int STAGE_X = 200;
// const int STAGE_Y = 50;
// const int STAGE_WIDTH = 480;
// const int STAGE_HEIGHT = 360;

// const int STAGE_BORDER_SIZE = 4;

const Color COLOR_WHITE = {255, 255, 255, 255};
const Color COLOR_BLACK = {0, 0, 0, 255};
const Color COLOR_RED = {255, 0, 0, 255};
const Color COLOR_GREEN = {0, 255, 0, 255};
const Color COLOR_BLUE = {0, 0, 255, 255};
const Color COLOR_GRAY = {128, 128, 128, 255};

const Color COLOR_MOTION = {74, 108, 212, 255};
const Color COLOR_CONTROL = {255, 171, 25, 255};
const Color COLOR_LOOKS = {148, 92, 213, 255};
const Color COLOR_SENSING = {92, 177, 214, 255};
const Color COLOR_OPERATORS = {89, 192, 89, 255};

#endif
