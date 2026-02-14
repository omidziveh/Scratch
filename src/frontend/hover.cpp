#include "hover.h"

void render_palette_hover(SDL_Renderer* renderer,
                          const std::vector<PaletteItem>& items,
                          int mouse_x, int mouse_y)
{
    for (const auto& item : items) {
        bool over = (mouse_x >= item.x &&
                     mouse_x <= item.x + item.width &&
                     mouse_y >= item.y &&
                     mouse_y <= item.y + item.height);
        if (over) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 40);
            SDL_Rect r = {
                (int)item.x,
                (int)item.y,
                (int)item.width,
                (int)item.height
            };
            SDL_RenderFillRect(renderer, &r);
        }
    }
}
