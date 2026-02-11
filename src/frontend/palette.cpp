#include "palette.h"
#include "draw.h"
#include "block_utils.h"
#include "../common/globals.h"

void init_palette(std::vector<PaletteItem>& items) {
    items.clear();

    float startX = (float)(PALETTE_X + 20);
    float startY = (float)(PALETTE_Y + 20);
    float gap    = 50.0f;
    float w      = (float)(PALETTE_WIDTH - 40);
    float h      = (float)BLOCK_HEIGHT;

    struct PaletteDef {
        BlockType type;
        std::string label;
    };

    PaletteDef defs[] = {
        {CMD_START,    "When START clicked"},
        {CMD_MOVE,     "Move (10) steps"},
        {CMD_TURN,     "Turn (15) degrees"},
        {CMD_GOTO,     "Go to x:(0) y:(0)"},
        {CMD_SET_X,    "Set x to (0)"},
        {CMD_SET_Y,    "Set y to (0)"},
        {CMD_CHANGE_X, "Change x by (10)"},
        {CMD_CHANGE_Y, "Change y by (10)"},
        {CMD_REPEAT,   "Repeat (10)"},
        {CMD_IF,       "If <> then"},
        {CMD_WAIT,     "Wait (1) secs"},
        {CMD_SAY,      "Say [Hello!]"},
    };

    int count = sizeof(defs) / sizeof(defs[0]);
    for (int i = 0; i < count; i++) {
        SDL_Color color = block_get_color(defs[i].type);
        PaletteItem item(defs[i].type, defs[i].label, color,
                         startX, startY + i * gap, w, h);
        items.push_back(item);
    }
}

void draw_palette(SDL_Renderer* renderer, const std::vector<PaletteItem>& items) {
    draw_filled_rect(renderer, PALETTE_X, PALETTE_Y, PALETTE_WIDTH, PALETTE_HEIGHT, COLOR_PALETTE_BG);

    for (const auto& item : items) {
        Block temp;
        temp.x = item.x;
        temp.y = item.y;
        temp.width = item.width;
        temp.height = item.height;
        temp.color = item.color;
        draw_block(renderer, temp, item.label);
    }
}
