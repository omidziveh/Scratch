#ifndef PEN_H
#define PEN_H

#include <SDL2/SDL.h>
#include "../common/definitions.h"

void pen_init(SDL_Renderer* renderer);
void pen_shutdown();
void pen_clear(SDL_Renderer* renderer);
void pen_stamp(SDL_Renderer* renderer, Sprite& sprite);
void pen_update(SDL_Renderer* renderer, Sprite& sprite);
void pen_render(SDL_Renderer* renderer);
void pen_set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void pen_set_size(int size);

#endif
