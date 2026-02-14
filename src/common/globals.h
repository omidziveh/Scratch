#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>

const SDL_Color COLOR_WHITE      = {255, 255, 255, 255};
const SDL_Color COLOR_BLACK      = {0,   0,   0,   255};
const SDL_Color COLOR_GRAY       = {200, 200, 200, 255};
const SDL_Color COLOR_DARK_GRAY  = {80,  80,  80,  255};
const SDL_Color COLOR_LIGHT_GRAY = {230, 230, 230, 255};
const SDL_Color COLOR_RED        = {255, 80,  80,  255};
const SDL_Color COLOR_GREEN      = {80,  200, 80,  255};
const SDL_Color COLOR_BLUE       = {80,  80,  255, 255};
const SDL_Color COLOR_YELLOW     = {255, 200, 50,  255};
const SDL_Color COLOR_ORANGE     = {255, 160, 50,  255};
const SDL_Color COLOR_PURPLE     = {160, 80,  255, 255};
const SDL_Color COLOR_CYAN       = {80,  200, 255, 255};

const SDL_Color COLOR_MOTION     = {76,  151, 255, 255};
const SDL_Color COLOR_CONTROL    = {255, 171, 25,  255};
const SDL_Color COLOR_LOOKS      = {153, 102, 255, 255};
const SDL_Color COLOR_EVENTS     = {255, 213, 0,   255};
const SDL_Color COLOR_SOUND      = {207, 99,  207, 255};
const SDL_Color COLOR_PEN        = {0,   171, 132, 255};

const SDL_Color COLOR_TOOLBAR_BG    = {60,  60,  60,  255};
const SDL_Color COLOR_PALETTE_BG    = {45,  45,  45,  255};
const SDL_Color COLOR_CODING_BG     = {30,  30,  30,  255};
const SDL_Color COLOR_STAGE_BG      = {255, 255, 255, 255};
const SDL_Color COLOR_STAGE_BORDER  = {180, 180, 180, 255};

extern int g_execution_index;
extern bool g_is_executing;
extern bool g_step_mode;
extern bool g_waiting_for_step;

#endif
