#ifndef PALETTE_H
#define PALETTE_H

#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>

void init_palette(std::vector<PaletteItem>& items);
void draw_palette(SDL_Renderer* renderer, const std::vector<PaletteItem>& items, int scroll_offset);
int get_palette_total_height(const std::vector<PaletteItem>& items);

#endif
