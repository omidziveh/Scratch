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
        {CMD_START,         "When START clicked"},
        {CMD_MOVE,          "Move (10) steps"},
        {CMD_TURN,          "Turn (15) degrees"},
        {CMD_GOTO,          "Go to x:(0) y:(0)"},
        {CMD_SET_X,         "Set x to (0)"},
        {CMD_SET_Y,         "Set y to (0)"},
        {CMD_CHANGE_X,      "Change x by (10)"},
        {CMD_CHANGE_Y,      "Change y by (10)"},
        {CMD_REPEAT,        "Repeat (10)"},
        {CMD_IF,            "If <> then"},
        {CMD_WAIT,          "Wait (1) secs"},
        {CMD_SAY,           "Say [Hello!]"},
        {CMD_SWITCH_COSTUME,"Switch costume to (1)"},
        {CMD_NEXT_COSTUME,  "Next costume"},
        {CMD_SET_SIZE,      "Set size to (100)%"},
        {CMD_CHANGE_SIZE,   "Change size by (10)"},
        {CMD_SHOW,          "Show"},
        {CMD_HIDE,          "Hide"},
        {CMD_PLAY_SOUND,    "Play sound"},
        {CMD_STOP_ALL_SOUNDS,"Stop all sounds"},
        {CMD_CHANGE_VOLUME, "Change volume by (10)"},
        {CMD_SET_VOLUME,    "Set volume to (100)"},
        {CMD_PEN_DOWN,      "Pen down"},
        {CMD_PEN_UP,        "Pen up"},
        {CMD_PEN_CLEAR,     "Clear pen"},
        {CMD_PEN_SET_COLOR, "Set pen color to (0)"},
        {CMD_PEN_SET_SIZE,  "Set pen size to (1)"},
        {CMD_PEN_STAMP,     "Stamp"},
        {OP_ADD,            "Add ( + )"},
        {OP_SUB,            "Sub ( - )"},
        {OP_DIV,            "Div ( / )"}
    };

    int count = sizeof(defs) / sizeof(defs[0]);
    for (int i = 0; i < count; i++) {
        SDL_Color color = block_get_color(defs[i].type);
        PaletteItem item(defs[i].type, defs[i].label, color,
                         startX, startY + i * gap, w, h);
        items.push_back(item);
    }
}

int get_palette_total_height(const std::vector<PaletteItem>& items) {
    if (items.empty()) return 0;
    float min_y = items[0].y;
    float max_y = items.back().y + items.back().height;
    return (int)(max_y - min_y + 40);
}

void draw_palette(SDL_Renderer* renderer, const std::vector<PaletteItem>& items, int scroll_offset) {
    draw_filled_rect(renderer, PALETTE_X, PALETTE_Y, PALETTE_WIDTH, PALETTE_HEIGHT, COLOR_PALETTE_BG);

    SDL_Rect clip_rect = {PALETTE_X, PALETTE_Y, PALETTE_WIDTH, PALETTE_HEIGHT};
    SDL_RenderSetClipRect(renderer, &clip_rect);

    for (const auto& item : items) {
        float draw_y = item.y - (float)scroll_offset;

        if (draw_y + item.height < (float)PALETTE_Y) continue;
        if (draw_y > (float)(PALETTE_Y + PALETTE_HEIGHT)) continue;

        Block temp;
        temp.x = item.x;
        temp.y = draw_y;
        temp.width = item.width;
        temp.height = item.height;
        temp.color = item.color;
        draw_block(renderer, temp, item.label);
    }

    SDL_RenderSetClipRect(renderer, nullptr);
}
