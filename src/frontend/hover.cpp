#include "hover.h"

void render_palette_hover(SDL_Renderer* renderer,
                          const std::vector<PaletteItem>& items,
                          int mouse_x, int mouse_y, int scroll_offset)
{
    for (const auto& item : items) {
        float draw_y = item.y - (float)scroll_offset;
        bool over = (mouse_x >= item.x &&
                     mouse_x <= item.x + item.width &&
                     mouse_y >= draw_y &&
                     mouse_y <= draw_y + item.height);
        if (over) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_Rect r = {
                (int)item.x,
                (int)draw_y,
                (int)item.width,
                (int)item.height
            };
            SDL_RenderFillRect(renderer, &r);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    }
}
