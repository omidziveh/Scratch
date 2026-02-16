#ifndef HOVER_H
#define HOVER_H

#include <SDL2/SDL.h>
#include "../gfx/SDL2_gfxPrimitives.h"
#include <vector>
#include "../common/definitions.h"

void render_palette_hover(SDL_Renderer* renderer,
                          const std::vector<PaletteItem>& items,
                          int mouse_x, int mouse_y, int scroll_offset);

void render_button_hover(SDL_Renderer* renderer, int bx, int by, int bw, int bh,
                          const char* label,
                          Uint8 nr, Uint8 ng, Uint8 nb,
                          int mouse_x, int mouse_y);

void render_play_pause_button(SDL_Renderer* renderer, bool is_running, bool is_paused,
                               int bx, int by, int bw, int bh,
                               int mouse_x, int mouse_y);

#endif
