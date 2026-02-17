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

    log_info(" glowing block: " + std::to_string(block.id));
    int bx = (int)block.x;
    int by = (int)block.y;
    int bw = (int)block.width;
    int bh = (int)block.height;

    Uint32 elapsed = SDL_GetTicks() - block.glow_start_time;
    float pulse = 0.5f + 0.5f * sinf(elapsed * 0.004f);
    Uint8 alpha = (Uint8)(60 + pulse * 140);

    for (int i = 6; i >= 1; i--) {
        Uint8 layer_alpha = (Uint8)(alpha * (1.0f - (float)i / 7.0f));
        roundedBoxRGBA(renderer,
            bx - i, by - i, bx + bw + i, by + bh + i,
            8 + i,
            255, 230, 60, (Uint8)(layer_alpha * 0.3f));
        roundedRectangleRGBA(renderer,
            bx - i, by - i, bx + bw + i, by + bh + i,
            8 + i,
            255, 255, 100, layer_alpha);
    }
}


void draw_text(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color) {
    if (!g_font) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
    int bh = isReporter ? 25 : BLOCK_HEIGHT;

    Uint8 r = block.color.r;
    Uint8 g = block.color.g;
    Uint8 b = block.color.b;

    if (block.is_running) {
        r = (Uint8)std::min(255, (int)r + 60);
        g = (Uint8)std::min(255, (int)g + 60);
        b = (Uint8)std::min(255, (int)b + 60);
    }

    std::string textToDraw = label.empty() ? block_get_label(block.type) : label;
    if (isReporter) {
        roundedBoxRGBA(renderer, bx, by, bx + bw, by + bh, 12, r, g, b, block.color.a);
        roundedRectangleRGBA(renderer, bx, by, bx + bw, by + bh, 12, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 255);
        draw_text(renderer, bx + 8, by + 8, textToDraw, COLOR_BLACK);
    } else {
        roundedBoxRGBA(renderer, bx, by, bx + bw, by + bh, 6, r, g, b, block.color.a);
        roundedRectangleRGBA(renderer, bx, by, bx + bw, by + bh, 8, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 255);

        std::string headerText = get_header_label(block.type);
        draw_text(renderer, bx + 8, by + 12, headerText, COLOR_BLACK);

        int totalH = get_total_height((Block*)&block);
        int bodyY = by + bh;
        int bodyH = totalH - bh;

        if (bodyH > 0) {
            SDL_Rect bodyRect = {bx, bodyY, bw, bodyH};
            SDL_SetRenderDrawColor(renderer, (Uint8)(r*0.9), (Uint8)(g*0.9), (Uint8)(b*0.9), 255);
            SDL_RenderFillRect(renderer, &bodyRect);
            SDL_SetRenderDrawColor(renderer, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 255);
            SDL_RenderDrawLine(renderer, bx, bodyY, bx, bodyY + bodyH);
            SDL_RenderDrawLine(renderer, bx + bw, bodyY, bx + bw, bodyY + bodyH);
            SDL_RenderDrawLine(renderer, bx, bodyY + bodyH, bx + bw, bodyY + bodyH);
            roundedBoxRGBA(renderer, bx, bodyY + bodyH - 20, bx + bw, bodyY + bodyH, 6, (Uint8)(r*0.9), (Uint8)(g*0.9), (Uint8)(b*0.9), 255);
            roundedRectangleRGBA(renderer, bx, bodyY + bodyH - 20, bx + bw, bodyY + bodyH, 6, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 255);
            int argCount = get_arg_count(block.type);
            int argAreaH = 0;

            if (block.inner && argCount > 0) {
                int lineY = bodyY + argAreaH;
                SDL_SetRenderDrawColor(renderer, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 150);
                SDL_RenderDrawLine(renderer, bx + 10, lineY, bx + bw - 10, lineY);
            }
        }
    }
}

static void draw_block_tree(SDL_Renderer* renderer, Block* block, const TextInputState& state) {
    if (!block) return;
    
    std::string label = block_get_label(block->type);

    draw_block(renderer, *block, label);
    if (!is_reporter_block(block->type)) {
        draw_arg_boxes(renderer, *block, state);
    }

    if (block->inner) {
        int innerY = block->y + BLOCK_HEIGHT + 5; 
        
        Block* child = block->inner;
        while(child) {
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

void draw_all_blocks(SDL_Renderer* renderer, const std::vector<Block>& blocks, const TextInputState& state) {
    std::vector<Block>& mutable_blocks = const_cast<std::vector<Block>&>(blocks);
    for (auto& block : mutable_blocks) {
        if (block.parent == nullptr) {
            draw_block_tree(renderer, &block, state);
        }
    }
}


void draw_toolbar(SDL_Renderer* renderer) {
    for (int row = TOOLBAR_Y; row < TOOLBAR_Y + TOOLBAR_HEIGHT; row++) {
        float t = (float)(row - TOOLBAR_Y) / (float)TOOLBAR_HEIGHT;
        Uint8 shade = (Uint8)(52 + t * 14);  // از 52 تا 66 - گرادیان ملایم
        SDL_SetRenderDrawColor(renderer, shade, shade, (Uint8)(shade + 5), 255);
        SDL_RenderDrawLine(renderer, TOOLBAR_X, row, TOOLBAR_X + TOOLBAR_WIDTH, row);
    }

    hlineRGBA(renderer,
        TOOLBAR_X, TOOLBAR_X + TOOLBAR_WIDTH,
        TOOLBAR_Y + TOOLBAR_HEIGHT - 1,
        75, 75, 90, 255);

    int startCX = TOOLBAR_WIDTH - 75;
    int startCY = TOOLBAR_Y + TOOLBAR_HEIGHT / 2;
    int btnRadius = 13;

    filledCircleRGBA(renderer, startCX + 1, startCY + 1, btnRadius, 0, 0, 0, 40);
    filledCircleRGBA(renderer, startCX, startCY, btnRadius, 75, 195, 75, 255);
    aacircleRGBA(renderer, startCX, startCY, btnRadius, 55, 160, 55, 255);
    filledTrigonRGBA(renderer,
        startCX - 4, startCY - 6,
        startCX - 4, startCY + 6,
        startCX + 7, startCY,
        255, 255, 255, 230);

    int stopCX = TOOLBAR_WIDTH - 40;
    int stopCY = TOOLBAR_Y + TOOLBAR_HEIGHT / 2;

    filledCircleRGBA(renderer, stopCX + 1, stopCY + 1, btnRadius, 0, 0, 0, 40);
    filledCircleRGBA(renderer, stopCX, stopCY, btnRadius, 220, 65, 55, 255);
    aacircleRGBA(renderer, stopCX, stopCY, btnRadius, 180, 50, 40, 255);
    boxRGBA(renderer,
        stopCX - 5, stopCY - 5,
        stopCX + 5, stopCY + 5,
        255, 255, 255, 230);
}

void draw_coding_area(SDL_Renderer* renderer) {
    SDL_Color bgColor = {30, 30, 40, 255};
    draw_filled_rect(renderer, CODING_AREA_X, CODING_AREA_Y,
                     CODING_AREA_WIDTH, CODING_AREA_HEIGHT, bgColor);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int dotSpacing = 22;
    for (int gx = CODING_AREA_X + 11; gx < CODING_AREA_X + CODING_AREA_WIDTH; gx += dotSpacing) {
        for (int gy = CODING_AREA_Y + 11; gy < CODING_AREA_Y + CODING_AREA_HEIGHT; gy += dotSpacing) {
            pixelRGBA(renderer, gx, gy, 65, 65, 80, 70);
        }
    }

    rectangleRGBA(renderer,
        CODING_AREA_X, CODING_AREA_Y,
        CODING_AREA_X + CODING_AREA_WIDTH, CODING_AREA_Y + CODING_AREA_HEIGHT,
        50, 50, 60, 200);
}


void draw_stage(SDL_Renderer* renderer, Sprite& sprite) {
    int sx = STAGE_X;
    int sy = STAGE_Y;
    int sw = STAGE_WIDTH;
    int sh = STAGE_HEIGHT;
    int radius = 8;

    roundedBoxRGBA(renderer,
        sx + 3, sy + 3, sx + sw + 3, sy + sh + 3,
        radius, 0, 0, 0, 45);

    roundedBoxRGBA(renderer,
        sx, sy, sx + sw, sy + sh,
        radius,
        COLOR_STAGE_BG.r, COLOR_STAGE_BG.g, COLOR_STAGE_BG.b, 255);

    draw_stage_border(renderer);

    draw_sprite(renderer, sprite);
}

void draw_stage_border(SDL_Renderer* renderer) {
    int sx = STAGE_X;
    int sy = STAGE_Y;
    int sw = STAGE_WIDTH;
    int sh = STAGE_HEIGHT;

    roundedRectangleRGBA(renderer,
        sx, sy, sx + sw, sy + sh,
        8,
        COLOR_STAGE_BORDER.r, COLOR_STAGE_BORDER.g,
        COLOR_STAGE_BORDER.b, COLOR_STAGE_BORDER.a);
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
        std::string argLbl = get_arg_label(block.type, i);
        if (!argLbl.empty()) {
            draw_text(renderer, box.x, box.y - 8, argLbl.c_str(), COLOR_GREEN);
        }
        bool has_block = (i < (int)block.argBlocks.size() && block.argBlocks[i] != nullptr);

        if (has_block) {
        
            Block* sub = block.argBlocks[i];
            
            sub->x = box.x;
            sub->y = box.y;
            sub->width = box.w;
            sub->height = box.h;
            std::string lbl = block_get_label(sub->type);
            
            {
                Uint8 r = sub->color.r;
                Uint8 g = sub->color.g;
                Uint8 b = sub->color.b;

                roundedBoxRGBA(renderer, box.x, box.y, box.x + box.w, box.y + box.h, 6, r, g, b, 255);
                roundedRectangleRGBA(renderer, box.x, box.y, box.x + box.w, box.y + box.h, 6, (Uint8)(r*0.7), (Uint8)(g*0.7), (Uint8)(b*0.7), 255);

                std::string valText;
                if (!sub->args.empty()) {
                    valText = sub->args[0];
                } else {
                    valText = get_header_label(sub->type);
                }

                int maxW = box.w - 10;
                if ((int)valText.length() * 8 > maxW) {
                    valText = valText.substr(0, (maxW / 8));
                }

                draw_text(renderer, box.x + 5, box.y + 8, valText.c_str(), COLOR_BLACK);
            }
        } else {
            bool is_editing = (state.active && state.block_id == block.id && state.arg_index == i);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &box);

            if (is_editing) SDL_SetRenderDrawColor(renderer, 50, 120, 255, 255);
            else SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            SDL_RenderDrawRect(renderer, &box);

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
                draw_text(renderer, text_x, text_y, display_text.c_str(), COLOR_BLACK);
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
}

void draw_category_bar(SDL_Renderer* renderer, const std::vector<CategoryItem>& categories, int selected_index) {
    int totalWidth = STAGE_X; 
    int buttonWidth = totalWidth / (int)categories.size();
    
    for (size_t i = 0; i < categories.size(); i++) {
        const auto& cat = categories[i];
        int x = PALETTE_X + i * buttonWidth;
        int y = CATEGORY_BAR_Y;
        
        SDL_Rect rect = {x, y, buttonWidth, CATEGORY_BAR_HEIGHT};
        
        SDL_Color col = cat.color;
        if ((int)i == selected_index) {
            col.r = std::min(255, col.r + 30);
            col.g = std::min(255, col.g + 30);
            col.b = std::min(255, col.b + 30);
        }
        
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &rect);
        draw_text(renderer, x + 5, y + 15, cat.name.c_str(), COLOR_BLACK);
    }
}