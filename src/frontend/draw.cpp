#include "draw.h"
#include "../common/globals.h"
#include <SDL2/SDL_image.h>

SDL_Texture* load_texture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void draw_stage_background(SDL_Renderer* renderer, Stage* stage) {
    SDL_SetRenderDrawColor(renderer,
        stage->background_color.r,
        stage->background_color.g,
        stage->background_color.b,
        stage->background_color.a);
    SDL_Rect bg_rect;
    bg_rect.x = stage->x;
    bg_rect.y = stage->y;
    bg_rect.w = stage->width;
    bg_rect.h = stage->height;
    SDL_RenderFillRect(renderer, &bg_rect);
}

void draw_stage_border(SDL_Renderer* renderer, Stage* stage) {
    SDL_SetRenderDrawColor(renderer,
        stage->border_color.r,
        stage->border_color.g,
        stage->border_color.b,
        stage->border_color.a);

    SDL_Rect top;
    top.x = stage->x - STAGE_BORDER_SIZE;
    top.y = stage->y - STAGE_BORDER_SIZE;
    top.w = stage->width + STAGE_BORDER_SIZE * 2;
    top.h = STAGE_BORDER_SIZE;
    SDL_RenderFillRect(renderer, &top);

    SDL_Rect bottom;
    bottom.x = stage->x - STAGE_BORDER_SIZE;
    bottom.y = stage->y + stage->height;
    bottom.w = stage->width + STAGE_BORDER_SIZE * 2;
    bottom.h = STAGE_BORDER_SIZE;
    SDL_RenderFillRect(renderer, &bottom);

    SDL_Rect left;
    left.x = stage->x - STAGE_BORDER_SIZE;
    left.y = stage->y;
    left.w = STAGE_BORDER_SIZE;
    left.h = stage->height;
    SDL_RenderFillRect(renderer, &left);

    SDL_Rect right_side;
    right_side.x = stage->x + stage->width;
    right_side.y = stage->y;
    right_side.w = STAGE_BORDER_SIZE;
    right_side.h = stage->height;
    SDL_RenderFillRect(renderer, &right_side);
}

void draw_sprite(SDL_Renderer* renderer, Sprite* sprite, Stage* stage) {
    if (sprite->visible == 0) {
        return;
    }
    if (sprite->texture == NULL) {
        return;
    }
    SDL_Rect dest_rect;
    dest_rect.x = stage->x + (int)sprite->x - (int)(sprite->width / 2);
    dest_rect.y = stage->y + (int)sprite->y - (int)(sprite->height / 2);
    dest_rect.w = (int)sprite->width;
    dest_rect.h = (int)sprite->height;

    SDL_Point center;
    center.x = (int)(sprite->width / 2);
    center.y = (int)(sprite->height / 2);

    SDL_RenderCopyEx(renderer,
        sprite->texture,
        NULL,
        &dest_rect,
        sprite->angle,
        &center,
        SDL_FLIP_NONE);
}
