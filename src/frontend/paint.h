#ifndef PALETTE_H
#define PALETTE_H

#include <SDL2/SDL.h>
#include <vector>
#include "../common/definitions.h"

void init_palette(std::vector<PaletteItem>& items);
void draw_palette(SDL_Renderer* renderer, const std::vector<PaletteItem>& items);

#endif
