#include "pen.h"
#include "../gfx/SDL2_gfxPrimitives.h"

#include <cmath>

static SDL_Texture* pen_canvas = nullptr;
static Uint8 pen_r = 0, pen_g = 0, pen_b = 200, pen_a = 255;
static int pen_thickness = 2;
static bool initialized = false;

void pen_init(SDL_Renderer* renderer) {
    if (pen_canvas) {
        SDL_DestroyTexture(pen_canvas);
    }
    pen_canvas = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        STAGE_WIDTH, STAGE_HEIGHT);
    SDL_SetTextureBlendMode(pen_canvas, SDL_BLENDMODE_BLEND);

    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pen_canvas);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, prev);

    initialized = true;
}

void pen_shutdown() {
    if (pen_canvas) {
        SDL_DestroyTexture(pen_canvas);
        pen_canvas = nullptr;
    }
    initialized = false;
}

void pen_clear(SDL_Renderer* renderer) {
    if (!pen_canvas) return;
    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pen_canvas);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, prev);
}

void pen_set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    pen_r = r;
    pen_g = g;
    pen_b = b;
    pen_a = a;
}

void pen_set_size(int size) {
    if (size < 1) size = 1;
    if (size > 50) size = 50;
    pen_thickness = size;
}

void pen_stamp(SDL_Renderer* renderer, Sprite& sprite) {
    if (!pen_canvas || !sprite.texture) return;

    SDL_Texture* prev = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pen_canvas);

    int draw_w = (int)(sprite.width * sprite.scale);
    int draw_h = (int)(sprite.height * sprite.scale);
    int sx = (int)(sprite.x - STAGE_X - draw_w / 2);
    int sy = (int)(sprite.y - STAGE_Y - draw_h / 2);

    SDL_Rect dst = { sx, sy, draw_w, draw_h };
    SDL_RenderCopyEx(renderer, sprite.texture, nullptr, &dst,
                     sprite.direction, nullptr, SDL_FLIP_NONE);

    SDL_SetRenderTarget(renderer, prev);
}

void pen_draw_line(SDL_Renderer* renderer, float x1, float y1,
                   float x2, float y2, const Sprite& sprite) {
    if (!pen_canvas) return;

    int cx1 = (int)(x1 - STAGE_X);
    int cy1 = (int)(y1 - STAGE_Y);
    int cx2 = (int)(x2 - STAGE_X);
    int cy2 = (int)(y2 - STAGE_Y);

    SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pen_canvas);

    Uint8 r = sprite.penR;
    Uint8 g = sprite.penG;
    Uint8 b = sprite.penB;
    int thickness = sprite.penSize;

    if (thickness <= 1) {
        aalineRGBA(renderer, cx1, cy1, cx2, cy2, r, g, b, 255);
    } else {
        thickLineRGBA(renderer, cx1, cy1, cx2, cy2, thickness, r, g, b, 255);
    }

    SDL_SetRenderTarget(renderer, prev_target);
}

void pen_update(SDL_Renderer* renderer, Sprite& sprite) {
    if (!pen_canvas || !sprite.isPenDown) return;

    float cx = sprite.x;
    float cy = sprite.y;
    float px = sprite.prevPenX;
    float py = sprite.prevPenY;

    float dx = cx - px;
    float dy = cy - py;
    if (dx * dx + dy * dy < 0.5f) return;

    SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, pen_canvas);

    int x1 = (int)(px - STAGE_X);
    int y1 = (int)(py - STAGE_Y);
    int x2 = (int)(cx - STAGE_X);
    int y2 = (int)(cy - STAGE_Y);

    if (pen_thickness <= 1) {
        aalineRGBA(renderer, x1, y1, x2, y2, pen_r, pen_g, pen_b, pen_a);
    } else {
        thickLineRGBA(renderer, x1, y1, x2, y2, pen_thickness,
                      pen_r, pen_g, pen_b, pen_a);
    }

    SDL_SetRenderTarget(renderer, prev_target);

    sprite.prevPenX = cx;
    sprite.prevPenY = cy;
}

void pen_render(SDL_Renderer* renderer) {
    if (!pen_canvas) return;
    SDL_Rect dst = { STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT };
    SDL_RenderCopy(renderer, pen_canvas, nullptr, &dst);
}
