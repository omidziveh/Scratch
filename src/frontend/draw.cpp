#include "draw.h"
#include "block_utils.h"
#include "text_input.h"
#include "../common/globals.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "../utils/logger.h"
#include "palette.h"
#include "block_utils.h"

static SDL_Color color_darken(SDL_Color c, float factor) {
    return {
        (Uint8)std::max(0, (int)(c.r * factor)),
        (Uint8)std::max(0, (int)(c.g * factor)),
        (Uint8)std::max(0, (int)(c.b * factor)),
        c.a
    };
}

static SDL_Color color_lighten(SDL_Color c, int amount) {
    return {
        (Uint8)std::min(255, c.r + amount),
        (Uint8)std::min(255, c.g + amount),
        (Uint8)std::min(255, c.b + amount),
        c.a
    };
}

static void draw_shadow_rounded(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
                                 int radius, int offsetX, int offsetY, int layers) {
    for (int i = layers; i >= 1; i--) {
        Uint8 a = (Uint8)(10 * (layers + 1 - i));
        roundedBoxRGBA(renderer,
            x1 + offsetX + i, y1 + offsetY + i,
            x2 + offsetX + i, y2 + offsetY + i,
            radius + i, 0, 0, 0, a);
    }
}

static void draw_notch_top(SDL_Renderer* renderer, int bx, int by, SDL_Color col) {
    int nx = bx + 14;
    Sint16 vx[] = {
        (Sint16)(nx),
        (Sint16)(nx + 4),
        (Sint16)(nx + 8),
        (Sint16)(nx + 14),
        (Sint16)(nx + 18),
        (Sint16)(nx + 22)
    };
    Sint16 vy[] = {
        (Sint16)(by),
        (Sint16)(by),
        (Sint16)(by - 4),
        (Sint16)(by - 4),
        (Sint16)(by),
        (Sint16)(by)
    };
    filledPolygonRGBA(renderer, vx, vy, 6, col.r, col.g, col.b, col.a);

    SDL_Color dk = color_darken(col, 0.7f);
    aapolygonRGBA(renderer, vx, vy, 6, dk.r, dk.g, dk.b, 120);
}

static void draw_notch_bottom(SDL_Renderer* renderer, int bx, int by_bottom, SDL_Color col) {
    int nx = bx + 14;
    Sint16 vx[] = {
        (Sint16)(nx),
        (Sint16)(nx + 4),
        (Sint16)(nx + 8),
        (Sint16)(nx + 14),
        (Sint16)(nx + 18),
        (Sint16)(nx + 22)
    };
    Sint16 vy[] = {
        (Sint16)(by_bottom),
        (Sint16)(by_bottom),
        (Sint16)(by_bottom + 4),
        (Sint16)(by_bottom + 4),
        (Sint16)(by_bottom),
        (Sint16)(by_bottom)
    };
    filledPolygonRGBA(renderer, vx, vy, 6, col.r, col.g, col.b, col.a);

    SDL_Color dk = color_darken(col, 0.7f);
    aapolygonRGBA(renderer, vx, vy, 6, dk.r, dk.g, dk.b, 80);
}

static void draw_block_3d_highlight(SDL_Renderer* renderer, int bx, int by, int bw, int bh, int radius) {
    roundedBoxRGBA(renderer,
        bx + 2, by + 1,
        bx + bw - 2, by + (bh / 3),
        radius, 255, 255, 255, 20);

    hlineRGBA(renderer, bx + radius, bx + bw - radius, by + bh - 1, 0, 0, 0, 30);
    hlineRGBA(renderer, bx + radius, bx + bw - radius, by + 1, 255, 255, 255, 15);
}

static void draw_text_shadowed(SDL_Renderer* renderer, int x, int y,
                                const std::string& text, SDL_Color color) {
    if (!g_font || text.empty()) return;

    SDL_Color shadow = {0, 0, 0, 60};
    SDL_Surface* sSh = TTF_RenderUTF8_Blended(g_font, text.c_str(), shadow);
    if (sSh) {
        SDL_Texture* tSh = SDL_CreateTextureFromSurface(renderer, sSh);
        if (tSh) {
            SDL_Rect dst = {x + 1, y + 1, sSh->w, sSh->h};
            SDL_RenderCopy(renderer, tSh, nullptr, &dst);
            SDL_DestroyTexture(tSh);
        }
        SDL_FreeSurface(sSh);
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(g_font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

static int measure_text_width(const std::string& text) {
    if (!g_font || text.empty()) return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(g_font, text.c_str(), &w, &h);
    return w;
}


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
    if (!block.is_running) return;

    log_info(" glowing block: " + std::to_string(block.id));
    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    int bh = (int)block.height;

    Uint32 elapsed = SDL_GetTicks() - block.glow_start_time;
    float pulse = 0.5f + 0.5f * sinf(elapsed * 0.005f);
    Uint8 alpha = (Uint8)(60 + pulse * 160);

    for (int i = 10; i >= 1; i--) {
        float layerF = 1.0f - (float)i / 11.0f;
        Uint8 la = (Uint8)(alpha * layerF);

        roundedBoxRGBA(renderer,
            bx - i, by - i, bx + bw + i, by + bh + i,
            8 + i, 255, 240, 70, (Uint8)(la * 0.2f));
        roundedRectangleRGBA(renderer,
            bx - i, by - i, bx + bw + i, by + bh + i,
            8 + i, 255, 255, 130, la);
    }
}

void draw_text(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color) {
    if (!g_font || text.empty()) return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(g_font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void draw_block(SDL_Renderer* renderer, const Block& block, const std::string& label) {
    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    bool isReporter = is_reporter_block(block.type);
    int bh = isReporter ? 26 : BLOCK_HEIGHT;

    SDL_Color base = block.color;
    if (block.is_running) {
        base = color_lighten(base, 50);
    }

    Uint8 r = base.r;
    Uint8 g = base.g;
    Uint8 b = base.b;
    SDL_Color dark = color_darken(base, 0.65f);
    SDL_Color darker = color_darken(base, 0.5f);

    std::string textToDraw = label.empty() ? block_get_label(block.type) : label;

    if (isReporter) {
        draw_shadow_rounded(renderer, bx, by, bx + bw, by + bh, 12, 2, 2, 3);

        roundedBoxRGBA(renderer, bx, by, bx + bw, by + bh, 12, r, g, b, base.a);

        roundedBoxRGBA(renderer,
            bx + 2, by + 1,
            bx + bw - 2, by + bh / 3,
            10, 255, 255, 255, 22);

        hlineRGBA(renderer, bx + 12, bx + bw - 12, by + bh - 1, 0, 0, 0, 28);

        roundedRectangleRGBA(renderer, bx, by, bx + bw, by + bh, 12,
                             dark.r, dark.g, dark.b, 255);

        draw_text_shadowed(renderer, bx + 10, by + 5, textToDraw, COLOR_WHITE);
        return;
    }

    draw_shadow_rounded(renderer, bx, by, bx + bw, by + bh, 6, 2, 3, 4);

    roundedBoxRGBA(renderer, bx, by, bx + bw, by + bh, 6, r, g, b, base.a);

    bool isEvent = (block.type == CMD_START || block.type == CMD_EVENT_CLICK);
    if (!isEvent) {
        draw_notch_top(renderer, bx, by, base);
    }
    draw_notch_bottom(renderer, bx, by + bh, base);

    draw_block_3d_highlight(renderer, bx, by, bw, bh, 6);

    roundedRectangleRGBA(renderer, bx, by, bx + bw, by + bh, 6,
                         dark.r, dark.g, dark.b, 200);

    std::string headerText = get_header_label(block.type);
    draw_text_shadowed(renderer, bx + 10, by + 10, headerText, COLOR_WHITE);

    int totalH = get_total_height((Block*)&block);
    int bodyY = by + bh;
    int bodyH = totalH - bh;

    if (bodyH > 0) {
        SDL_Color bodyCol = color_darken(base, 0.85f);

        roundedBoxRGBA(renderer, bx, bodyY, bx + bw, bodyY + bodyH, 0,
                       bodyCol.r, bodyCol.g, bodyCol.b, 255);

        int innerX = bx + 14;
        int innerY = bodyY + 5;
        int innerW = bw - 28;
        int innerH = bodyH - 30;
        if (innerH > 6) {
            roundedBoxRGBA(renderer,
                innerX, innerY, innerX + innerW, innerY + innerH,
                5, darker.r, darker.g, darker.b, 120);

            roundedRectangleRGBA(renderer,
                innerX, innerY, innerX + innerW, innerY + innerH,
                5, dark.r, dark.g, dark.b, 70);

            hlineRGBA(renderer, innerX + 3, innerX + innerW - 3, innerY + 1, 0, 0, 0, 25);
        }

        vlineRGBA(renderer, bx, bodyY, bodyY + bodyH, dark.r, dark.g, dark.b, 200);
        vlineRGBA(renderer, bx + bw, bodyY, bodyY + bodyH, dark.r, dark.g, dark.b, 200);

        int footerH = 22;
        int footerY = bodyY + bodyH - footerH;

        roundedBoxRGBA(renderer, bx, footerY, bx + bw, bodyY + bodyH,
                       6, r, g, b, base.a);

        draw_notch_bottom(renderer, bx, bodyY + bodyH, base);

        draw_block_3d_highlight(renderer, bx, footerY, bw, footerH, 6);

        roundedRectangleRGBA(renderer, bx, footerY, bx + bw, bodyY + bodyH,
                             6, dark.r, dark.g, dark.b, 200);

        int argCount = get_arg_count(block.type);
        if (block.inner && argCount > 0) {
            int argAreaH = 0;
            int lineY = bodyY + argAreaH;
            SDL_SetRenderDrawColor(renderer, dark.r, dark.g, dark.b, 100);
            SDL_RenderDrawLine(renderer, bx + 14, lineY, bx + bw - 14, lineY);
        }
    }
}

static void draw_block_tree(SDL_Renderer* renderer, Block* block, const TextInputState& state) {
    if (!block) return;

    std::string label = block_get_label(block->type);

    draw_block_glow(renderer, *block);
    draw_block(renderer, *block, label);

    if (!is_reporter_block(block->type)) {
        draw_arg_boxes(renderer, *block, state);
    }

    if (block->inner) {
        int innerY = block->y + BLOCK_HEIGHT + 5;
        Block* child = block->inner;
        while (child) {
            child->x = block->x + 15;
            child->y = innerY;
            draw_block_tree(renderer, child, state);
            innerY += get_total_height(child);
            child = child->next;
        }
    }

    if (block->next) {
        block->next->x = block->x;
        block->next->y = block->y + get_total_height(block);
        draw_block_tree(renderer, block->next, state);
    }
}

void draw_all_blocks(SDL_Renderer* renderer, const std::list<Block>& blocks, const TextInputState& state) {
    std::list<Block>& mutable_blocks = const_cast<std::list<Block>&>(blocks);
    for (auto& block : mutable_blocks) {
        if (block.parent == nullptr) {
            draw_block_tree(renderer, &block, state);
        }
    }
}

void draw_toolbar(SDL_Renderer* renderer) {
    for (int row = TOOLBAR_Y; row < TOOLBAR_Y + TOOLBAR_HEIGHT; row++) {
        float t = (float)(row - TOOLBAR_Y) / (float)TOOLBAR_HEIGHT;
        float curve = t * t;
        Uint8 shade = (Uint8)(44 + curve * 22);
        SDL_SetRenderDrawColor(renderer, shade, shade, (Uint8)(shade + 10), 255);
        SDL_RenderDrawLine(renderer, TOOLBAR_X, row, TOOLBAR_X + TOOLBAR_WIDTH, row);
    }

    hlineRGBA(renderer, TOOLBAR_X, TOOLBAR_X + TOOLBAR_WIDTH, TOOLBAR_Y, 65, 65, 80, 100);
    hlineRGBA(renderer, TOOLBAR_X, TOOLBAR_X + TOOLBAR_WIDTH, TOOLBAR_Y + TOOLBAR_HEIGHT - 1, 80, 80, 100, 220);

    int startCX = TOOLBAR_WIDTH - 75;
    int startCY = TOOLBAR_Y + TOOLBAR_HEIGHT / 2;
    int btnR = 15;

    filledCircleRGBA(renderer, startCX + 2, startCY + 3, btnR + 2, 0, 0, 0, 50);
    filledCircleRGBA(renderer, startCX, startCY, btnR + 2, 40, 155, 40, 255);
    filledCircleRGBA(renderer, startCX, startCY, btnR, 75, 200, 75, 255);
    filledCircleRGBA(renderer, startCX, startCY - 4, btnR - 5, 120, 230, 120, 55);
    aacircleRGBA(renderer, startCX, startCY, btnR + 2, 30, 130, 30, 255);
    aacircleRGBA(renderer, startCX, startCY, btnR, 90, 215, 90, 120);

    filledTrigonRGBA(renderer,
        startCX - 5, startCY - 8,
        startCX - 5, startCY + 8,
        startCX + 8, startCY,
        255, 255, 255, 245);
    aatrigonRGBA(renderer,
        startCX - 5, startCY - 8,
        startCX - 5, startCY + 8,
        startCX + 8, startCY,
        255, 255, 255, 80);

    int stopCX = TOOLBAR_WIDTH - 40;
    int stopCY = TOOLBAR_Y + TOOLBAR_HEIGHT / 2;

    filledCircleRGBA(renderer, stopCX + 2, stopCY + 3, btnR + 2, 0, 0, 0, 50);
    filledCircleRGBA(renderer, stopCX, stopCY, btnR + 2, 175, 35, 30, 255);
    filledCircleRGBA(renderer, stopCX, stopCY, btnR, 220, 62, 52, 255);
    filledCircleRGBA(renderer, stopCX, stopCY - 4, btnR - 5, 245, 105, 95, 50);
    aacircleRGBA(renderer, stopCX, stopCY, btnR + 2, 145, 25, 20, 255);
    aacircleRGBA(renderer, stopCX, stopCY, btnR, 235, 80, 70, 120);

    roundedBoxRGBA(renderer,
        stopCX - 5, stopCY - 5,
        stopCX + 5, stopCY + 5,
        2, 255, 255, 255, 245);
}

void draw_coding_area(SDL_Renderer* renderer) {
    for (int row = CODING_AREA_Y; row < CODING_AREA_Y + CODING_AREA_HEIGHT; row++) {
        float t = (float)(row - CODING_AREA_Y) / (float)CODING_AREA_HEIGHT;
        Uint8 base = (Uint8)(24 + t * 12);
        SDL_SetRenderDrawColor(renderer, base, base, (Uint8)(base + 10), 255);
        SDL_RenderDrawLine(renderer, CODING_AREA_X, row,
                           CODING_AREA_X + CODING_AREA_WIDTH, row);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int dotSpacing = 24;
    for (int gx = CODING_AREA_X + 12; gx < CODING_AREA_X + CODING_AREA_WIDTH; gx += dotSpacing) {
        for (int gy = CODING_AREA_Y + 12; gy < CODING_AREA_Y + CODING_AREA_HEIGHT; gy += dotSpacing) {
            hlineRGBA(renderer, gx - 1, gx + 1, gy, 58, 58, 78, 50);
            vlineRGBA(renderer, gx, gy - 1, gy + 1, 58, 58, 78, 50);
        }
    }

    for (int i = 0; i < 3; i++) {
        Uint8 sa = (Uint8)(50 - i * 15);
        hlineRGBA(renderer,
            CODING_AREA_X, CODING_AREA_X + CODING_AREA_WIDTH,
            CODING_AREA_Y + i, 0, 0, 0, sa);
        vlineRGBA(renderer,
            CODING_AREA_X + i, CODING_AREA_Y, CODING_AREA_Y + CODING_AREA_HEIGHT,
            0, 0, 0, (Uint8)(sa * 0.7f));
    }

    rectangleRGBA(renderer,
        CODING_AREA_X, CODING_AREA_Y,
        CODING_AREA_X + CODING_AREA_WIDTH, CODING_AREA_Y + CODING_AREA_HEIGHT,
        42, 42, 55, 180);
}

void draw_stage(SDL_Renderer* renderer, Sprite& sprite) {
    int sx = STAGE_X;
    int sy = STAGE_Y;
    int sw = STAGE_WIDTH;
    int sh = STAGE_HEIGHT;
    int radius = 10;

    for (int i = 6; i >= 1; i--) {
        Uint8 a = (Uint8)(10 * (7 - i));
        roundedBoxRGBA(renderer,
            sx + i + 1, sy + i + 2,
            sx + sw + i + 1, sy + sh + i + 2,
            radius + i, 0, 0, 0, a);
    }

    roundedBoxRGBA(renderer, sx, sy, sx + sw, sy + sh, radius,
                   COLOR_STAGE_BG.r, COLOR_STAGE_BG.g, COLOR_STAGE_BG.b, 255);

    hlineRGBA(renderer, sx + 14, sx + sw - 14, sy + 1, 255, 255, 255, 22);

    draw_stage_border(renderer);
    draw_sprite(renderer, sprite);
}

void draw_stage_border(SDL_Renderer* renderer) {
    int sx = STAGE_X;
    int sy = STAGE_Y;
    int sw = STAGE_WIDTH;
    int sh = STAGE_HEIGHT;

    roundedRectangleRGBA(renderer, sx, sy, sx + sw, sy + sh, 10,
                         COLOR_STAGE_BORDER.r, COLOR_STAGE_BORDER.g,
                         COLOR_STAGE_BORDER.b, COLOR_STAGE_BORDER.a);

    roundedRectangleRGBA(renderer, sx + 1, sy + 1, sx + sw - 1, sy + sh - 1, 9,
                         255, 255, 255, 12);
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
    SDL_RenderCopyEx(renderer, sprite.texture, nullptr, &dest, sprite.angle, nullptr, SDL_FLIP_NONE);
    SDL_RenderSetClipRect(renderer, nullptr);
}

void draw_cursor(SDL_Renderer* renderer, int x, int y, int height, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x, y, x, y + height);
    SDL_RenderDrawLine(renderer, x + 1, y, x + 1, y + height);
}

void draw_arg_boxes(SDL_Renderer* renderer, const Block& block, const TextInputState& state) {
    int arg_count = get_arg_count(block.type);
    if (arg_count == 0) return;

    for (int i = 0; i < arg_count; i++) {
        SDL_Rect box = get_arg_box_rect(block, i);
        if (box.w == 0) continue;

        std::string argLbl = get_arg_label(block.type, i);
        if (!argLbl.empty()) {
            draw_text(renderer, box.x, box.y - 10, argLbl.c_str(), COLOR_GREEN);
        }

        bool has_block = (i < (int)block.argBlocks.size() && block.argBlocks[i] != nullptr);

        if (has_block) {
            Block* sub = block.argBlocks[i];
            sub->x = box.x;
            sub->y = box.y;
            sub->width = box.w;
            sub->height = box.h;

            SDL_Color sc = sub->color;
            SDL_Color sd = color_darken(sc, 0.65f);

            roundedBoxRGBA(renderer,
                box.x + 1, box.y + 2,
                box.x + box.w + 1, box.y + box.h + 2,
                10, 0, 0, 0, 35);

            roundedBoxRGBA(renderer,
                box.x, box.y, box.x + box.w, box.y + box.h,
                10, sc.r, sc.g, sc.b, 255);

            roundedBoxRGBA(renderer,
                box.x + 2, box.y + 1,
                box.x + box.w - 2, box.y + box.h / 3,
                8, 255, 255, 255, 22);

            roundedRectangleRGBA(renderer,
                box.x, box.y, box.x + box.w, box.y + box.h,
                10, sd.r, sd.g, sd.b, 255);

            std::string valText;
            if (!sub->args.empty()) {
                valText = sub->args[0];
            } else {
                valText = get_header_label(sub->type);
            }

            int maxW = box.w - 14;
            if ((int)valText.length() * 8 > maxW && maxW > 0) {
                valText = valText.substr(0, (maxW / 8));
            }

            draw_text_shadowed(renderer, box.x + 7, box.y + 5, valText.c_str(), COLOR_WHITE);

        } else {
            bool is_editing = (state.active && state.block_id == block.id && state.arg_index == i);

            roundedBoxRGBA(renderer,
                box.x, box.y, box.x + box.w, box.y + box.h,
                5, 248, 248, 252, 255);

            hlineRGBA(renderer, box.x + 4, box.x + box.w - 4, box.y + 1, 0, 0, 0, 18);

            if (is_editing) {
                roundedRectangleRGBA(renderer,
                    box.x - 2, box.y - 2,
                    box.x + box.w + 2, box.y + box.h + 2,
                    7, 60, 130, 255, 70);
                roundedRectangleRGBA(renderer,
                    box.x - 1, box.y - 1,
                    box.x + box.w + 1, box.y + box.h + 1,
                    6, 65, 140, 255, 255);
                roundedRectangleRGBA(renderer,
                    box.x, box.y, box.x + box.w, box.y + box.h,
                    5, 80, 155, 255, 180);
            } else {
                roundedRectangleRGBA(renderer,
                    box.x, box.y, box.x + box.w, box.y + box.h,
                    5, 170, 170, 182, 255);
            }

            std::string display_text;
            if (is_editing) {
                display_text = state.buffer;
            } else if (i < (int)block.args.size()) {
                display_text = block.args[i];
            }

            if (!display_text.empty()) {
                int text_x = box.x + 5;
                int text_y = box.y + (box.h - 10) / 2;

                int max_chars = (box.w - 10) / 8;
                if ((int)display_text.size() > max_chars && max_chars > 0) {
                    display_text = display_text.substr(0, max_chars);
                }
                draw_text(renderer, text_x, text_y, display_text.c_str(), {25, 25, 35, 255});
            }

            if (is_editing && state.cursor_visible) {
                int cursor_x = box.x + 5 + state.cursor_pos * 8;
                if (cursor_x > box.x + box.w - 4) {
                    cursor_x = box.x + box.w - 4;
                }
                draw_cursor(renderer, cursor_x, box.y + 3, box.h - 6, {25, 25, 180, 255});
            }
        }
    }
}

void draw_category_bar(SDL_Renderer* renderer, const std::vector<CategoryItem>& categories, int selected_index) {
    int totalWidth = STAGE_X;
    int buttonWidth = totalWidth / (int)categories.size();

    boxRGBA(renderer,
        PALETTE_X, CATEGORY_BAR_Y,
        PALETTE_X + totalWidth, CATEGORY_BAR_Y + CATEGORY_BAR_HEIGHT,
        38, 38, 48, 255);

    for (int row = CATEGORY_BAR_Y; row < CATEGORY_BAR_Y + 3; row++) {
        Uint8 a = (Uint8)(30 - (row - CATEGORY_BAR_Y) * 10);
        hlineRGBA(renderer, PALETTE_X, PALETTE_X + totalWidth, row, 0, 0, 0, a);
    }

    hlineRGBA(renderer,
        PALETTE_X, PALETTE_X + totalWidth,
        CATEGORY_BAR_Y + CATEGORY_BAR_HEIGHT - 1,
        55, 55, 70, 220);

    for (size_t i = 0; i < categories.size(); i++) {
        const auto& cat = categories[i];
        int x = PALETTE_X + (int)i * buttonWidth;
        int y = CATEGORY_BAR_Y;

        SDL_Color col = cat.color;

        if ((int)i == selected_index) {
            SDL_Color bright = color_lighten(col, 30);

            roundedBoxRGBA(renderer,
                x + 1, y + 2,
                x + buttonWidth - 1, y + CATEGORY_BAR_HEIGHT,
                4, bright.r, bright.g, bright.b, 255);

            roundedBoxRGBA(renderer,
                x + 2, y + 3,
                x + buttonWidth - 2, y + CATEGORY_BAR_HEIGHT / 2,
                3, 255, 255, 255, 28);

            boxRGBA(renderer,
                x + 6, y + CATEGORY_BAR_HEIGHT - 3,
                x + buttonWidth - 6, y + CATEGORY_BAR_HEIGHT,
                255, 255, 255, 200);

            int tw = measure_text_width(cat.name);
            int tx = x + (buttonWidth - tw) / 2;
            draw_text_shadowed(renderer, tx, y + 11, cat.name.c_str(), COLOR_WHITE);

        } else {
            roundedBoxRGBA(renderer,
                x + 2, y + 5,
                x + buttonWidth - 2, y + CATEGORY_BAR_HEIGHT - 2,
                3, col.r, col.g, col.b, 120);

            int tw = measure_text_width(cat.name);
            int tx = x + (buttonWidth - tw) / 2;
            draw_text(renderer, tx, y + 12, cat.name.c_str(), {195, 195, 210, 210});
        }

        if (i < categories.size() - 1) {
            vlineRGBA(renderer,
                x + buttonWidth,
                y + 8, y + CATEGORY_BAR_HEIGHT - 8,
                65, 65, 80, 90);
        }
    }
}
