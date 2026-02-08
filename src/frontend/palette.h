#ifndef PALETTE_H
#define PALETTE_H

#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>

namespace BlockCoding {

extern std::vector<PaletteItem> palette_items;

void init_palette();
void draw_palette(SDL_Renderer* renderer);
BlockType get_clicked_palette_item(int mouseX, int mouseY);
bool is_in_palette_area(int x, int y);

}

#endif
