#ifndef PALETTE_H
#define PALETTE_H

#include "../common/definitions.h"
#include <SDL2/SDL.h>

void init_palette();
void draw_palette(SDL_Renderer* renderer);
int get_clicked_palette_item(int x, int y);
BlockType get_palette_block_type(int index);

#endif
