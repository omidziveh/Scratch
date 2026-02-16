#include "hover.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <cmath>

void render_palette_hover(SDL_Renderer* renderer,
                          const std::vector<PaletteItem>& items,
                          int mouse_x, int mouse_y, int scroll_offset) {
    for (size_t i = 0; i < items.size(); i++) {
        float item_y = items[i].y - (float)scroll_offset;
        bool hovered = (mouse_x >= items[i].x &&
                        mouse_x <= items[i].x + items[i].width &&
                        mouse_y >= item_y &&
                        mouse_y <= item_y + items[i].height);
        if (hovered) {
            SDL_Rect highlight = {
                (int)items[i].x - 2,
                (int)item_y - 2,
                (int)items[i].width + 4,
                (int)items[i].height + 4
            };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
            SDL_RenderFillRect(renderer, &highlight);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
            SDL_RenderDrawRect(renderer, &highlight);
        }
    }
}

void render_button_hover(SDL_Renderer* renderer, int bx, int by, int bw, int bh,
                          const char* label,
                          Uint8 nr, Uint8 ng, Uint8 nb,
                          int mouse_x, int mouse_y) {
    bool hovered = (mouse_x >= bx && mouse_x <= bx + bw &&
                    mouse_y >= by && mouse_y <= by + bh);
    SDL_Rect btn = {bx, by, bw, bh};

    if (hovered) {
        Uint8 hr = (Uint8)fmin(nr + 35, 255);
        Uint8 hg = (Uint8)fmin(ng + 35, 255);
        Uint8 hb = (Uint8)fmin(nb + 35, 255);
        SDL_SetRenderDrawColor(renderer, hr, hg, hb, 255);
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
        SDL_RenderDrawRect(renderer, &btn);
    } else {
        SDL_SetRenderDrawColor(renderer, nr, ng, nb, 255);
        SDL_RenderFillRect(renderer, &btn);
    }
    stringRGBA(renderer, bx + 5, by + (bh - 8) / 2, label, 255, 255, 255, 255);
}

void render_play_pause_button(SDL_Renderer* renderer, bool is_running, bool is_paused,
                               int bx, int by, int bw, int bh,
                               int mouse_x, int mouse_y) {
    bool hovered = (mouse_x >= bx && mouse_x <= bx + bw &&
                    mouse_y >= by && mouse_y <= by + bh);
    SDL_Rect btn = {bx, by, bw, bh};

    if (is_running && !is_paused) {
        Uint8 r = hovered ? 255 : 220;
        Uint8 g = hovered ? 180 : 150;
        SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect bar1 = {bx + bw / 2 - 7, by + 6, 5, bh - 12};
        SDL_Rect bar2 = {bx + bw / 2 + 2, by + 6, 5, bh - 12};
        SDL_RenderFillRect(renderer, &bar1);
        SDL_RenderFillRect(renderer, &bar2);
    } else {
        Uint8 g = hovered ? 200 : 175;
        SDL_SetRenderDrawColor(renderer, hovered ? 90 : 76, g, hovered ? 90 : 80, 255);
        SDL_RenderFillRect(renderer, &btn);
        filledTrigonRGBA(renderer, bx + bw / 2 - 6, by + 5, bx + bw / 2 - 6, by + bh - 5, bx + bw / 2 + 8, by + bh / 2, 255, 255, 255, 255);
    }

    if (hovered) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
        SDL_RenderDrawRect(renderer, &btn);
    }
}
