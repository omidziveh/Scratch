#pragma once
#include <SDL2/SDL.h>
#include "../common/globals.h"
#include "../common/definitions.h"

extern SDL_Window* window;
extern SDL_Renderer* renderer;

// Graphics functions
bool init_graphics();
void shutdown_graphics();
void clear(int r, int g, int b);
void present();
void fill_rect(int x, int y, int w, int h);
void set_color(int r, int g, int b);

// Block drawing
void draw_block(Block* b);
