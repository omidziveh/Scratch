
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef SDL_DISABLE_ANALYZE_MACROS
#define SDL_DISABLE_ANALYZE_MACROS 1
#endif

#include <string>
#include <vector>
#include <SDL2/SDL.h>

const int DEFAULT_TICK_RATE = 60;
const int DEFAULT_MAX_TICKS = 60;
const int DEFAULT_WATCHDOG_THRESHOLD = 1000;
const int LOOP_WATCHDOG_LIMIT = 1000;

const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;

const int STAGE_X      = 800;
const int STAGE_Y      = 70;
const int STAGE_WIDTH  = 460;
const int STAGE_HEIGHT = 340;

const int PALETTE_X      = 0;
const int PALETTE_Y      = 70;
const int PALETTE_WIDTH  = 200;
const int PALETTE_HEIGHT = 680;

const int CODING_AREA_X      = 200;
const int CODING_AREA_Y      = 70;
const int CODING_AREA_WIDTH  = 600;
const int CODING_AREA_HEIGHT = 680;

const int TOOLBAR_X      = 0;
const int TOOLBAR_Y      = 30;
const int TOOLBAR_WIDTH  = 1280;
const int TOOLBAR_HEIGHT = 40;

const int BLOCK_WIDTH  = 200;
const int BLOCK_HEIGHT = 40;
const int SNAP_DISTANCE = 20;

const int ARG_BOX_WIDTH    = 45;
const int ARG_BOX_HEIGHT   = 20;
const int ARG_BOX_MARGIN_X = 8;
const int ARG_BOX_Y_OFFSET = 10;
const Uint32 CURSOR_BLINK_MS = 500;

const int CATEGORY_BAR_Y = 30;
const int CATEGORY_BAR_HEIGHT = 40;

const int MENU_BAR_OFFSET = 30;

const int EVENT_BLOCKS_COUNT = 0;
const int MOTION_BLOCKS_COUNT = 1;
const int CONTROL_BLOCKS_COUNT = 8;
const int LOOKS_BLOCKS_COUNT = 11;
const int SOUND_BLOCKS_COUNT = 18;
const int PEN_BLOCKS_COUNT = 22;
const int OPERATORS_BLOCKS_COUNT = 28;


struct Color {
    Uint8 r, g, b, a;
};

struct Costume {
    std::string name;
    SDL_Texture* texture;
    int width;
    int height;

    Costume()
        : name("")
        , texture(nullptr)
        , width(0)
        , height(0)
    {}

    Costume(const std::string& n, SDL_Texture* tex, int w, int h)
        : name(n)
        , texture(tex)
        , width(w)
        , height(h)
    {}
};

struct Stage {
    int x;
    int y;
    int width;
    int height;
    Color border_color;
    Color background_color;
    SDL_Renderer* renderer;
    SDL_Texture* penCanvas;

    Stage()
        : x(STAGE_X)
        , y(STAGE_Y)
        , width(STAGE_WIDTH)
        , height(STAGE_HEIGHT)
        , border_color({100, 100, 100, 255})
        , background_color({255, 255, 255, 255})
        , renderer(nullptr)
        , penCanvas(nullptr)
    {}
};


struct Sprite {
    float x;
    float y;
    int width;
    int height;
    float angle;
    float direction;
    int visible;
    int isPenDown;
    int currentCostumeIndex;
    SDL_Texture* texture;
    std::string name;
    float scale;
    float volume;
    std::vector<Costume> costumes;
    float prevPenX;
    float prevPenY;
    bool penMoved;
    Uint8 penR;
    Uint8 penG;
    Uint8 penB;
    int penSize;
    std::string sayText;
    Uint32 sayStartTime;
    float sayDuration; 
    Sprite()
        : x(STAGE_X + STAGE_WIDTH / 2.0f)
        , y(STAGE_Y + STAGE_HEIGHT / 2.0f)
        , width(80)
        , height(80)
        , angle(0.0f)
        , direction(0.0f)
        , visible(1)
        , isPenDown(0)
        , currentCostumeIndex(0)
        , texture(nullptr)
        , name("Cat")
        , scale(0.5f)
        , volume(100)
        , prevPenX(STAGE_X + STAGE_WIDTH / 2.0f)
        , prevPenY(STAGE_Y + STAGE_HEIGHT / 2.0f)
        , penMoved(false)
        , penR(0)
        , penG(0)
        , penB(255)
        , penSize(1)
    {}
};

enum BlockType {
    CMD_NONE = 0,
    CMD_MOVE,
    CMD_TURN,
    CMD_GOTO,
    CMD_SET_X,
    CMD_SET_Y,
    CMD_CHANGE_X,
    CMD_CHANGE_Y,
    CMD_REPEAT,
    CMD_IF,
    CMD_WAIT,
    CMD_SAY,
    CMD_START,
    CMD_EVENT_CLICK,
    CMD_SWITCH_COSTUME,
    CMD_NEXT_COSTUME,
    CMD_SET_SIZE,
    CMD_CHANGE_SIZE,
    CMD_SHOW,
    CMD_HIDE,
    CMD_PLAY_SOUND,
    CMD_STOP_ALL_SOUNDS,
    CMD_CHANGE_VOLUME,
    CMD_SET_VOLUME,
    SENSE_TOUCHING_MOUSE,
    SENSE_TOUCHING_EDGE,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    CMD_PEN_DOWN,
    CMD_PEN_UP,
    CMD_PEN_CLEAR,
    CMD_PEN_SET_COLOR,
    CMD_PEN_SET_SIZE,
    CMD_PEN_STAMP

};

struct Block {
    int id;
    BlockType type;
    float x, y;
    float width, height;
    bool dragging;
    bool hasBreakpoint;
    float drag_offset_x;
    float drag_offset_y;
    std::vector<std::string> args;
    std::vector<std::string> argBlocks;
    SDL_Color color;

    Block* parent;
    bool is_snapped;
    Block* next;
    Block* inner;

    bool is_running;
    Uint32 glow_start_time;

    Block()
        : id(0)
        , type(CMD_NONE)
        , x(0), y(0)
        , width(BLOCK_WIDTH), height(BLOCK_HEIGHT)
        , dragging(false)
        , hasBreakpoint(false)
        , drag_offset_x(0), drag_offset_y(0)
        , color({100, 100, 255, 255})
        , parent(nullptr)
        , is_snapped(false)
        , next(nullptr)
        , inner(nullptr)
        , is_running(false)
        , glow_start_time(0)
    {}
};

struct PaletteItem {
    BlockType type;
    std::string label;
    SDL_Color color;
    float x, y;
    float width, height;

    PaletteItem()
        : type(CMD_NONE)
        , label("")
        , color({100, 100, 255, 255})
        , x(0), y(0)
        , width(BLOCK_WIDTH), height(BLOCK_HEIGHT)
    {}

    PaletteItem(BlockType t, const std::string& lbl, SDL_Color c, float px, float py, float w, float h)
        : type(t)
        , label(lbl)
        , color(c)
        , x(px), y(py)
        , width(w), height(h)
    {}
};

struct TextInputState {
    bool active;
    int block_id;
    int arg_index;
    std::string buffer;
    int cursor_pos;
    Uint32 blink_timer;
    bool cursor_visible;

    TextInputState()
        : active(false)
        , block_id(-1)
        , arg_index(0)
        , buffer("")
        , cursor_pos(0)
        , blink_timer(0)
        , cursor_visible(true)
    {}
};


struct ExecutionContext {
    Sprite* sprite;
    Stage*  stage;
    int     mouseX;
    int     mouseY;
    float   lastResult;
    bool    lastCondition;

    ExecutionContext()
        : sprite(nullptr)
        , stage(nullptr)
        , mouseX(0)
        , mouseY(0)
        , lastResult(0.0f)
        , lastCondition(false)
    {}
};

enum BlockCategory {
    CAT_NONE = 0,
    CAT_EVENTS,
    CAT_MOTION,
    CAT_CONTROL,
    CAT_LOOKS,
    CAT_SOUND,
    CAT_PEN,
    CAT_OPERATORS
};

#endif
