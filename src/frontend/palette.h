#ifndef PALETTE_H
#define PALETTE_H

#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>

struct CategoryItem {
    BlockCategory category;
    std::string name;
    SDL_Color color;
    float yPosition;
};

void init_palette(std::vector<PaletteItem>& items);
void init_categories();
const std::vector<CategoryItem>& get_categories();
int get_category_scroll_target(BlockCategory cat);
void draw_palette(SDL_Renderer* renderer, const std::vector<PaletteItem>& items, int scroll_offset);
int get_palette_total_height(const std::vector<PaletteItem>& items);

#endif
