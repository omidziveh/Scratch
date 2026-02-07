#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <SDL2/SDL.h>

struct Sprite {
    float x;
    float y;
    float width;
    float height;
    double angle;
    SDL_Texture* texture;
    int visible;
};

struct Stage {
    int x;
    int y;
    int width;
    int height;
    SDL_Color border_color;
    SDL_Color background_color;
};

#endif
