#include "palette.h"
#include "../gfx/SDL2_gfxPrimitives.h"

static PaletteItem palette_items[6];
static int palette_count = 0;

void init_palette() {
    palette_count = 6;
    int y_start = 20;
    int spacing = 60;

    palette_items[0] = PaletteItem();
    palette_items[0].type = BLOCK_MOVE;
    palette_items[0].x = 10;
    palette_items[0].y = y_start;
    palette_items[0].width = 130;
    palette_items[0].height = 50;
    palette_items[0].r = 66;
    palette_items[0].g = 133;
    palette_items[0].b = 244;
    palette_items[0].label = "Move";

    palette_items[1] = PaletteItem();
    palette_items[1].type = BLOCK_TURN;
    palette_items[1].x = 10;
    palette_items[1].y = y_start + spacing;
    palette_items[1].width = 130;
    palette_items[1].height = 50;
    palette_items[1].r = 33;
    palette_items[1].g = 100;
    palette_items[1].b = 200;
    palette_items[1].label = "Turn";

    palette_items[2] = PaletteItem();
    palette_items[2].type = BLOCK_REPEAT;
    palette_items[2].x = 10;
    palette_items[2].y = y_start + spacing * 2;
    palette_items[2].width = 130;
    palette_items[2].height = 50;
    palette_items[2].r = 255;
    palette_items[2].g = 152;
    palette_items[2].b = 0;
    palette_items[2].label = "Repeat";

    palette_items[3] = PaletteItem();
    palette_items[3].type = BLOCK_IF;
    palette_items[3].x = 10;
    palette_items[3].y = y_start + spacing * 3;
    palette_items[3].width = 130;
    palette_items[3].height = 50;
    palette_items[3].r = 255;
    palette_items[3].g = 193;
    palette_items[3].b = 7;
    palette_items[3].label = "If";

    palette_items[4] = PaletteItem();
    palette_items[4].type = BLOCK_WAIT;
    palette_items[4].x = 10;
    palette_items[4].y = y_start + spacing * 4;
    palette_items[4].width = 130;
    palette_items[4].height = 50;
    palette_items[4].r = 156;
    palette_items[4].g = 39;
    palette_items[4].b = 176;
    palette_items[4].label = "Wait";

    palette_items[5] = PaletteItem();
    palette_items[5].type = BLOCK_START;
    palette_items[5].x = 10;
    palette_items[5].y = y_start + spacing * 5;
    palette_items[5].width = 130;
    palette_items[5].height = 50;
    palette_items[5].r = 76;
    palette_items[5].g = 175;
    palette_items[5].b = 80;
    palette_items[5].label = "Start";
}

void draw_palette(SDL_Renderer* renderer) {
    boxRGBA(renderer, 0, 0, PALETTE_WIDTH, WINDOW_HEIGHT, 50, 50, 50, 255);
    lineRGBA(renderer, PALETTE_WIDTH, 0, PALETTE_WIDTH, WINDOW_HEIGHT, 100, 100, 100, 255);
    for (int i = 0; i < palette_count; i++) {
        PaletteItem& item = palette_items[i];
        roundedBoxRGBA(renderer,
            item.x, item.y,
            item.x + item.width, item.y + item.height,
            8, item.r, item.g, item.b, 255);
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
    return -1;
}

BlockType get_palette_block_type(int index) {
    if (index >= 0 && index < palette_count) {
        return palette_items[index].type;
    }
    return BLOCK_NONE;
}
