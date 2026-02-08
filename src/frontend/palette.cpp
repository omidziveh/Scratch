#include "palette.h"
#include "SDL2_gfxPrimitives.h"
#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>
#include <cstring>

namespace BlockCoding {

std::vector<PaletteItem> palette_items;

void init_palette() {
    palette_items.clear();
    
    int startY = PALETTE_Y + 20;
    int itemHeight = 50;
    
    PaletteItem moveItem;
    moveItem.type = BLOCK_MOVE;
    moveItem.x = PALETTE_X + 10;
    moveItem.y = startY;
    moveItem.width = PALETTE_WIDTH - 20;
    moveItem.height = itemHeight;
    palette_items.push_back(moveItem);
    
    PaletteItem turnItem;
    turnItem.type = BLOCK_TURN;
    turnItem.x = PALETTE_X + 10;
    turnItem.y = startY + PALETTE_BLOCK_SPACING;
    turnItem.width = PALETTE_WIDTH - 20;
    turnItem.height = itemHeight;
    palette_items.push_back(turnItem);
    
    PaletteItem repeatItem;
    repeatItem.type = BLOCK_REPEAT;
    repeatItem.x = PALETTE_X + 10;
    repeatItem.y = startY + PALETTE_BLOCK_SPACING * 2;
    repeatItem.width = PALETTE_WIDTH - 20;
    repeatItem.height = itemHeight;
    palette_items.push_back(repeatItem);
    
    PaletteItem ifItem;
    ifItem.type = BLOCK_IF;
    ifItem.x = PALETTE_X + 10;
    ifItem.y = startY + PALETTE_BLOCK_SPACING * 3;
    ifItem.width = PALETTE_WIDTH - 20;
    ifItem.height = itemHeight;
    palette_items.push_back(ifItem);
    
    PaletteItem waitItem;
    waitItem.type = BLOCK_WAIT;
    waitItem.x = PALETTE_X + 10;
    waitItem.y = startY + PALETTE_BLOCK_SPACING * 4;
    waitItem.width = PALETTE_WIDTH - 20;
    waitItem.height = itemHeight;
    palette_items.push_back(waitItem);
    
    PaletteItem eventItem;
    eventItem.type = BLOCK_EVENT_START;
    eventItem.x = PALETTE_X + 10;
    eventItem.y = startY + PALETTE_BLOCK_SPACING * 5;
    eventItem.width = PALETTE_WIDTH - 20;
    eventItem.height = itemHeight;
    palette_items.push_back(eventItem);
}

void draw_palette(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_Rect paletteRect = {PALETTE_X, PALETTE_Y, PALETTE_WIDTH, 500};
    SDL_RenderFillRect(renderer, &paletteRect);
    
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &paletteRect);
    
    for (const PaletteItem& item : palette_items) {
        SDL_Color color;
        const char* label;
        
        switch (item.type) {
            case BLOCK_MOVE:
                color = {66, 135, 245, 255};
                label = "Move";
                break;
            case BLOCK_TURN:
                color = {66, 135, 245, 255};
                label = "Turn";
                break;
            case BLOCK_REPEAT:
                color = {230, 150, 30, 255};
                label = "Repeat";
                break;
            case BLOCK_IF:
                color = {230, 150, 30, 255};
                label = "If";
                break;
            case BLOCK_WAIT:
                color = {230, 150, 30, 255};
                label = "Wait";
                break;
            case BLOCK_EVENT_START:
                color = {220, 180, 50, 255};
                label = "Start";
                break;
            default:
                color = {150, 150, 150, 255};
                label = "Block";
        }
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect blockRect = {item.x, item.y, item.width, item.height};
        SDL_RenderFillRect(renderer, &blockRect);
        
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer, &blockRect);
        
        stringRGBA(renderer, item.x + 10, item.y + 18, label, 255, 255, 255, 255);
    }
}

BlockType get_clicked_palette_item(int mouseX, int mouseY) {
    for (const PaletteItem& item : palette_items) {
        if (mouseX >= item.x && mouseX <= item.x + item.width &&
            mouseY >= item.y && mouseY <= item.y + item.height) {
            return item.type;
        }
    }
    return BLOCK_NONE;
}

bool is_in_palette_area(int x, int y) {
    return (x >= PALETTE_X && x <= PALETTE_X + PALETTE_WIDTH && y >= PALETTE_Y);
}

}
