#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL.h>
#include "../common/definitions.h"

SDL_Texture* load_texture(SDL_Renderer* renderer, const char* path);
void draw_stage_background(SDL_Renderer* renderer, Stage* stage);
void draw_stage_border(SDL_Renderer* renderer, Stage* stage);
void draw_sprite(SDL_Renderer* renderer, Sprite* sprite, Stage* stage);

#endif
