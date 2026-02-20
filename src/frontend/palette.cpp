#include "palette.h"
#include "draw.h"
#include "block_utils.h"
#include "../common/globals.h"

static std::vector<CategoryItem> g_categories;

BlockCategory get_block_category(BlockType type) {
    switch (type) {
        case CMD_START: return CAT_EVENTS;
        case CMD_MOVE:
        case CMD_TURN:
        case CMD_GOTO:
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
            return CAT_MOTION;
        case CMD_REPEAT:
        case CMD_IF:
        case CMD_WAIT:
            return CAT_CONTROL;
        case CMD_SAY:
        case CMD_SWITCH_COSTUME:
        case CMD_NEXT_COSTUME:
        case CMD_SET_SIZE:
        case CMD_CHANGE_SIZE:
        case CMD_SHOW:
        case CMD_HIDE:
            return CAT_LOOKS;
        case CMD_PLAY_SOUND:
        case CMD_STOP_ALL_SOUNDS:
        case CMD_CHANGE_VOLUME:
        case CMD_SET_VOLUME:
            return CAT_SOUND;
        case CMD_PEN_DOWN:
        case CMD_PEN_UP:
        case CMD_PEN_CLEAR:
        case CMD_PEN_SET_COLOR:
        case CMD_PEN_SET_SIZE:
        case CMD_PEN_STAMP:
            return CAT_PEN;
        case OP_ADD:
        case OP_SUB:
        case CMD_GOTO_RANDOM:       
        case CMD_GOTO_MOUSE:           
        case CMD_IF_ON_EDGE_BOUNCE:   
            return CAT_MOTION;
        case OP_DIV:
            return CAT_OPERATORS;
        case OP_MUL:
        case CMD_SET_VAR:
        case CMD_CHANGE_VAR:
            return CAT_VARIABLES;
        case SENSE_DISTANCE_TO_MOUSE: 
            return CAT_SENSING;
        default:
            return CAT_NONE;
    }
}

void init_categories() {
    g_categories.clear();
    g_categories.push_back({CAT_EVENTS,    "Events",    COLOR_EVENTS,   EVENT_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_MOTION,    "Motion",    COLOR_MOTION,   MOTION_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_CONTROL,   "Control",   COLOR_CONTROL,  CONTROL_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_LOOKS,     "Looks",     COLOR_LOOKS,    LOOKS_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_SOUND,     "Sound",     COLOR_SOUND,    SOUND_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_PEN,       "Pen",       COLOR_PEN,      PEN_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_SENSING,   "Sensing",   COLOR_SENSING,  SENSING_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_OPERATORS, "Operators", COLOR_OPERATOR, OPERATORS_BLOCKS_COUNT * BLOCK_HEIGHT});
    g_categories.push_back({CAT_VARIABLES, "Variables", COLOR_VARIABLE, VARIABLE_BLOCKS_COUNT * BLOCK_HEIGHT});
}

const std::vector<CategoryItem>& get_categories() {
    return g_categories;
}

int get_category_scroll_target(BlockCategory cat) {
    for (const auto& c : g_categories) {
        if (c.category == cat) return (int)c.yPosition;
    }
    return 0;
}

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
        {CMD_EVENT_CLICK,   "When this sprite clicked"},
        {CMD_EVENT_KEY,     "When [space] key pressed"},
        {CMD_MOVE,          "Move (10) steps"},
        {CMD_TURN,          "Turn (15) degrees"},
        {CMD_GOTO,          "Go to x:(0) y:(0)"},
        {CMD_SET_X,         "Set x to (0)"},
        {CMD_SET_Y,         "Set y to (0)"},
        {CMD_CHANGE_X,      "Change x by (10)"},
        {CMD_CHANGE_Y,      "Change y by (10)"},
        {CMD_GOTO_RANDOM,       "Go to random pos"},     
        {CMD_GOTO_MOUSE,        "Go to mouser"},       
        {CMD_IF_ON_EDGE_BOUNCE, "If on edge, bounce"},   
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
        {SENSE_MOUSE_DOWN,  "mouse down?"},
        {SENSE_MOUSE_X,     "mouse x"},
        {SENSE_MOUSE_Y,     "mouse y"},
        {SENSE_TIMER,       "timer"},
        {SENSE_RESET_TIMER, "reset timer"},
        {SENSE_DISTANCE_TO_MOUSE,"Dist to mouse"}, 
        {OP_ADD,        "( + )"},
        {OP_SUB,        "( - )"},
        {OP_MUL,        "( * )"},
        {OP_DIV,        "( / )"},
        {OP_GT,         "( > )"},
        {OP_LT,         "( < )"},
        {OP_EQ,         "( = )"},
        {OP_AND,        "( and )"},
        {OP_OR,         "( or )"},
        {OP_NOT,        "not ( )"},
        {OP_STR_LEN,    "length ( )"},
        {OP_STR_CHAR,   "letter ( ) of ( )"},
        {OP_STR_CONCAT, "join ( ) ( )"},
        {OP_MOD,        "mod ( )"},
        {OP_ABS,        "abs ( )"},
        {OP_SQRT,       "sqrt ( )"},
        {CMD_SET_VAR,     "Set [var] to (0)"},
        {CMD_CHANGE_VAR,  "Change [var] by (1)"},
        {CMD_DEFINE_BLOCK, "Define Block"},
        {CMD_CALL_BLOCK,   "Call Block"}
    };

    int count = sizeof(defs) / sizeof(defs[0]);
    init_categories();

    BlockCategory currentCat = CAT_NONE;
    float firstY = 0;

    for (int i = 0; i < count; i++) {
        SDL_Color color = block_get_color(defs[i].type);
        PaletteItem item(defs[i].type, defs[i].label, color,
                         startX, startY + i * gap, w, h);
        items.push_back(item);

        BlockCategory itemCat = get_block_category(defs[i].type);
        if (itemCat != currentCat) {
            currentCat = itemCat;
            float relativeY = (startY + i * gap) - PALETTE_Y;
            
            for (auto& cat : g_categories) {
                if (cat.category == currentCat) {
                    cat.yPosition = relativeY;
                    break;
                }
            }
        }
    }
}

int get_palette_total_height(const std::vector<PaletteItem>& items) {
    if (items.empty()) return 0;
    float min_y = items[0].y;
    float max_y = items.back().y + items.back().height;
    return (int)(max_y - min_y + 150);
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
        temp.type = item.type;
        temp.x = item.x;
        temp.y = draw_y;
        temp.width = item.width;
        temp.height = item.height;
        temp.color = item.color;
        draw_block(renderer, temp, item.label);
    }

    SDL_RenderSetClipRect(renderer, nullptr);
}
