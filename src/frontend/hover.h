#ifndef HOVER_H
#define HOVER_H

#include <SDL2/SDL.h>
#include <vector>
#include "../common/definitions.h"

void render_palette_hover(SDL_Renderer* renderer,
                          const std::vector<PaletteItem>& items,
                          int mouse_x, int mouse_y);

#endif
