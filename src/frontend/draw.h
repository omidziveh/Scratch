#pragma once
#include <SDL2/SDL.h>

// Global Graphics
extern SDL_Window* window;
extern SDL_Renderer* renderer;

// Functions
bool init_graphics();
void shutdown_graphics();
void clear(int r, int g, int b);
void render();
void set_color(int r, int g, int b);
void fill_rect(int x, int y, int w, int h);
void draw_rect(int x, int y, int w, int h);
