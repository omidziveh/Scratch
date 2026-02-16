#include "block_highlight.h"
#include <cmath>

void draw_block_execution_highlight(SDL_Renderer* renderer, Block* block, Uint32 current_time) {
    if (!block || !block->is_running) return;

    if (block->glow_start_time == 0) {
        block->glow_start_time = current_time;
    }

    SDL_Rect rect = {(int)block->x, (int)block->y, (int)block->width, (int)block->height};

    Uint32 elapsed = current_time - block->glow_start_time;
    int alpha = 150 + (int)(105.0f * sinf(elapsed * 0.005f));
    if (alpha > 255) alpha = 255;
    if (alpha < 100) alpha = 100;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 1; i <= 4; i++) {
        SDL_Rect glow = {rect.x - i, rect.y - i, rect.w + 2 * i, rect.h + 2 * i};
        SDL_SetRenderDrawColor(renderer, 255, 220, 0, alpha / (i + 1));
        SDL_RenderDrawRect(renderer, &glow);
    }

    Uint8 bright_r = (Uint8)fmin(block->color.r + 50, 255);
    Uint8 bright_g = (Uint8)fmin(block->color.g + 50, 255);
    Uint8 bright_b = (Uint8)fmin(block->color.b + 50, 255);

    SDL_SetRenderDrawColor(renderer, bright_r, bright_g, bright_b, 255);
    SDL_RenderFillRect(renderer, &rect);

    stringRGBA(renderer, rect.x + rect.w - 14, rect.y + 3, ">", 255, 255, 0, 255);
}

void draw_block_breakpoint_marker(SDL_Renderer* renderer, Block* block) {
    if (!block || !block->hasBreakpoint) return;

    SDL_Rect rect = {(int)block->x, (int)block->y, (int)block->width, (int)block->height};

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < 2; i++) {
        SDL_Rect bp = {rect.x - i, rect.y - i, rect.w + 2 * i, rect.h + 2 * i};
        SDL_RenderDrawRect(renderer, &bp);
    }

    filledCircleRGBA(renderer, rect.x + 6, rect.y + (int)(block->height / 2), 4, 255, 0, 0, 255);
}
