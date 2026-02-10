#include "palette.h"
#include "../gfx/SDL2_gfxPrimitives.h"

namespace BlockCoding {

    static PaletteItem palette_items[6];
    static int palette_count = 0;

    void init_palette() {
        palette_count = 6;
        
        int y_offset = 20;
        int spacing = 60;
        
        palette_items[0] = {BLOCK_MOVE, 10, y_offset, 130, 50, 66, 133, 244, "Move"};
        
        // Turn block - آبی تیره
        palette_items[1] = {BLOCK_TURN, 10, y_offset + spacing, 130, 50, 33, 100, 200, "Turn"};
        
        // Repeat block - نارنجی
        palette_items[2] = {BLOCK_REPEAT, 10, y_offset + spacing*2, 130, 50, 255, 152, 0, "Repeat"};
        
        // If block - زرد
        palette_items[3] = {BLOCK_IF, 10, y_offset + spacing*3, 130, 50, 255, 193, 7, "If"};
        
        // Wait block - بنفش
        palette_items[4] = {BLOCK_WAIT, 10, y_offset + spacing*4, 130, 50, 156, 39, 176, "Wait"};
        
        // Start block - سبز
        palette_items[5] = {BLOCK_START, 10, y_offset + spacing*5, 130, 50, 76, 175, 80, "Start"};
    }

    void draw_palette(SDL_Renderer* renderer) {
        // پس‌زمینه palette
        boxRGBA(renderer, 0, 0, PALETTE_WIDTH, 600, 50, 50, 50, 255);
        
        // خط جداکننده
        lineRGBA(renderer, PALETTE_WIDTH, 0, PALETTE_WIDTH, 600, 100, 100, 100, 255);
        
        // رسم هر آیتم
        for (int i = 0; i < palette_count; i++) {
            PaletteItem& item = palette_items[i];
            roundedBoxRGBA(renderer, 
                item.x, item.y, 
                item.x + item.width, item.y + item.height,
                8, item.r, item.g, item.b, 255);
            
            // متن (اختیاری - نیاز به SDL_ttf داره)
            stringRGBA(renderer, item.x + 10, item.y + 18, item.label, 255, 255, 255, 255);
        }
    }

    int get_clicked_palette_item(int x, int y) {
        for (int i = 0; i < palette_count; i++) {
            PaletteItem& item = palette_items[i];
            if (x >= item.x && x <= item.x + item.width &&
                y >= item.y && y <= item.y + item.height) {
                return i;
            }
        }
        return -1;  // هیچ آیتمی کلیک نشده
    }

    BlockType get_palette_block_type(int index) {
        if (index >= 0 && index < palette_count) {
            return palette_items[index].type;
        }
        return BLOCK_MOVE;  // پیش‌فرض
    }

}
