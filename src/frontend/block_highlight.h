#ifndef BLOCK_HIGHLIGHT_H
#define BLOCK_HIGHLIGHT_H
#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include "../gfx/SDL2_gfxPrimitives.h"

#include <cmath>

void draw_block_execution_highlight(SDL_Renderer* renderer, Block* block, Uint32 current_time);
void draw_block_breakpoint_marker(SDL_Renderer* renderer, Block* block);

#endif
