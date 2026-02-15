#include "draw.h"
#include "block_utils.h"
#include "text_input.h"
#include "../common/globals.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cmath>
#include <algorithm>


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
void draw_block_glow(SDL_Renderer* renderer, const Block& block) {
    if (!block.is_running)
        return;

    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    int bh = (int)block.height;

    Uint32 elapsed = SDL_GetTicks() - block.glow_start_time;
    float pulse = 0.5f + 0.5f * sinf(elapsed * 0.005f);
    Uint8 alpha = (Uint8)(80 + pulse * 120);

    for (int i = 4; i >= 1; i--) {
        Uint8 layer_alpha = (Uint8)(alpha * (1.0f - (float)i / 5.0f));
        roundedRectangleRGBA(renderer,
            bx - i, by - i, bx + bw + i, by + bh + i,
            6 + i,
            255, 255, 100, layer_alpha);
    }
}


void draw_block(SDL_Renderer* renderer, const Block& block, const std::string& label) {
    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    int bh = (int)block.height;

    Uint8 r = block.color.r;
    Uint8 g = block.color.g;
    Uint8 b = block.color.b;
    if (block.is_running) {
        r = (Uint8)std::min(255, (int)r + 60);
        g = (Uint8)std::min(255, (int)g + 60);
        b = (Uint8)std::min(255, (int)b + 60);
    }

    roundedBoxRGBA(renderer,
        bx, by, bx + bw, by + bh,
        6,
        r, g, b, block.color.a);

    Uint8 dr = (Uint8)(r * 0.7f);
    Uint8 dg = (Uint8)(g * 0.7f);
    Uint8 db = (Uint8)(b * 0.7f);
    roundedRectangleRGBA(renderer,
        bx, by, bx + bw, by + bh,
        6,
        dr, dg, db, 255);

    draw_text(renderer, bx + 8, by + 12, label, COLOR_WHITE);
}

void draw_all_blocks(SDL_Renderer* renderer, const std::vector<Block>& blocks, const TextInputState& state) {
    for (const auto& block : blocks) {
        std::string label = block_get_label(block.type);
        draw_block(renderer, block, label);
        draw_arg_boxes(renderer, block, state);
    }
}


void draw_toolbar(SDL_Renderer* renderer) {
    draw_filled_rect(renderer, TOOLBAR_X, TOOLBAR_Y, TOOLBAR_WIDTH, TOOLBAR_HEIGHT, COLOR_TOOLBAR_BG);
    draw_filled_rect(renderer, TOOLBAR_WIDTH - 90, TOOLBAR_Y + 5, 30, 30, COLOR_GREEN);
    draw_filled_rect(renderer, TOOLBAR_WIDTH - 50, TOOLBAR_Y + 5, 30, 30, COLOR_RED);
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

    SDL_Point center = {w / 2, h / 2};

    SDL_Rect stageClip = {STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT};
    SDL_RenderSetClipRect(renderer, &stageClip);
    SDL_RenderCopyEx(renderer, sprite.texture, nullptr, &dest, sprite.angle, nullptr, SDL_FLIP_NONE);
    SDL_RenderSetClipRect(renderer, nullptr);
}

void draw_text(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color) {
    stringRGBA(renderer, x, y, text.c_str(), color.r, color.g, color.b, color.a);
}
void draw_cursor(SDL_Renderer* renderer, int x, int y, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x, y, x, y + height);
}

void draw_arg_boxes(SDL_Renderer* renderer, const Block& block, const TextInputState& state) {
    int arg_count = get_arg_count(block.type);
    if (arg_count == 0) return;

    for (int i = 0; i < arg_count; i++) {
        SDL_Rect box = get_arg_box_rect(block, i);
        if (box.w == 0) continue;

        bool is_editing = (state.active &&
                           state.block_id == block.id &&
                           state.arg_index == i);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect fill_rect = {box.x, box.y, box.w, box.h};
        SDL_RenderFillRect(renderer, &fill_rect);

        if (is_editing) {
            SDL_SetRenderDrawColor(renderer, 50, 120, 255, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        }
        SDL_RenderDrawRect(renderer, &fill_rect);

        std::string display_text;
        if (is_editing) {
            display_text = state.buffer;
        } else {
            if (i < (int)block.args.size()) {
                display_text = block.args[i];
            }
        }

        if (!display_text.empty()) {
            int text_x = box.x + 3;
            int text_y = box.y + (box.h - 8) / 2;

            int max_chars = (box.w - 6) / 8;
            if ((int)display_text.size() > max_chars && max_chars > 0) {
                display_text = display_text.substr(0, max_chars);
            }

            stringRGBA(renderer, text_x, text_y,
                       display_text.c_str(),
                       0, 0, 0, 255);
        }

        if (is_editing && state.cursor_visible) {
            int cursor_x = box.x + 3 + state.cursor_pos * 8;
            if (cursor_x > box.x + box.w - 3) {
                cursor_x = box.x + box.w - 3;
            }
            draw_cursor(renderer, cursor_x, box.y + 2, box.h - 4, {0, 0, 0, 255});
        }
    }
}
