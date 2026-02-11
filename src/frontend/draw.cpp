#include "draw.h"
#include "block_utils.h"
#include "../common/globals.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <SDL_image.h>
#include <iostream>

SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    }
    return texture;
}

void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}

void draw_block(SDL_Renderer* renderer, const Block& block, const std::string& label) {
    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    int bh = (int)block.height;

    roundedBoxRGBA(renderer,
        bx, by, bx + bw, by + bh,
        6,
        block.color.r, block.color.g, block.color.b, block.color.a);

    Uint8 dr = (Uint8)(block.color.r * 0.7f);
    Uint8 dg = (Uint8)(block.color.g * 0.7f);
    Uint8 db = (Uint8)(block.color.b * 0.7f);
    roundedRectangleRGBA(renderer,
        bx, by, bx + bw, by + bh,
        6,
        dr, dg, db, 255);

    draw_text(renderer, bx + 8, by + 12, label, COLOR_WHITE);
}

void draw_all_blocks(SDL_Renderer* renderer, const std::vector<Block>& blocks) {
    for (const auto& block : blocks) {
        std::string label = block_get_label(block.type);
        draw_block(renderer, block, label);
    }
}

void draw_toolbar(SDL_Renderer* renderer) {
    draw_filled_rect(renderer, TOOLBAR_X, TOOLBAR_Y, TOOLBAR_WIDTH, TOOLBAR_HEIGHT, COLOR_TOOLBAR_BG);
    draw_filled_rect(renderer, TOOLBAR_WIDTH - 90, 5, 30, 30, COLOR_GREEN);
    draw_filled_rect(renderer, TOOLBAR_WIDTH - 50, 5, 30, 30, COLOR_RED);
}

void draw_coding_area(SDL_Renderer* renderer) {
    draw_filled_rect(renderer, CODING_AREA_X, CODING_AREA_Y, CODING_AREA_WIDTH, CODING_AREA_HEIGHT, COLOR_CODING_BG);
    draw_rect_outline(renderer, CODING_AREA_X, CODING_AREA_Y, CODING_AREA_WIDTH, CODING_AREA_HEIGHT, COLOR_DARK_GRAY);
}

void draw_stage(SDL_Renderer* renderer, Sprite& sprite) {
    draw_filled_rect(renderer, STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT, COLOR_STAGE_BG);
    draw_stage_border(renderer);
    draw_sprite(renderer, sprite);
}

void draw_stage_border(SDL_Renderer* renderer) {
    draw_rect_outline(renderer, STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT, COLOR_STAGE_BORDER);
}

void draw_sprite(SDL_Renderer* renderer, Sprite& sprite) {
    if (!sprite.visible || !sprite.texture) return;

    int w, h;
    SDL_QueryTexture(sprite.texture, nullptr, nullptr, &w, &h);

    int draw_w = (int)(w * sprite.scale);
    int draw_h = (int)(h * sprite.scale);

    SDL_Rect dest;
    dest.x = (int)(sprite.x - draw_w / 2.0f);
    dest.y = (int)(sprite.y - draw_h / 2.0f);
    dest.w = draw_w;
    dest.h = draw_h;

    SDL_Rect stageClip = {STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT};
    SDL_RenderSetClipRect(renderer, &stageClip);
    SDL_RenderCopyEx(renderer, sprite.texture, nullptr, &dest, sprite.direction, nullptr, SDL_FLIP_NONE);
    SDL_RenderSetClipRect(renderer, nullptr);
}

void draw_text(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color) {
    stringRGBA(renderer, x, y, text.c_str(), color.r, color.g, color.b, color.a);
}
