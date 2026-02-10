#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

const int BLOCK_WIDTH = 50;
const int BLOCK_HEIGHT = 40;

const int STAGE_X = 800;
const int STAGE_Y = 50;
const int STAGE_WIDTH = 380;
const int STAGE_HEIGHT = 300;
const int STAGE_BORDER_SIZE = 3;

const int PALETTE_WIDTH = 150;
const int PALETTE_X = 0;
const int PALETTE_Y = 50;
const int PALETTE_BLOCK_SPACING = 70;
const int WORKSPACE_X = PALETTE_WIDTH + 10;

const int DEFAULT_TICK_RATE = 30;
const int DEFAULT_MAX_TICKS = 10000;

const int MIN_BLOCK_DATA_COUNT = 9;
const int MIN_SPRITE_DATA_COUNT = 5;

struct Block;
extern std::vector<Block*> all_blocks;

enum BlockType {
    BLOCK_NONE = 0,
    CMD_MOVE,
    CMD_TURN,
    CMD_GOTO,
    CMD_REPEAT,
    CMD_IF,
    CMD_WAIT,
    CMD_SAY,
    CMD_EVENT_CLICK,
    SENSE_TOUCHING_MOUSE,
    SENSE_TOUCHING_EDGE,
    OP_ADD,
    OP_SUB,
    OP_DIV,
    BLOCK_MOVE = CMD_MOVE,
    BLOCK_TURN = CMD_TURN,
    BLOCK_GOTO = CMD_GOTO,
    BLOCK_REPEAT = CMD_REPEAT,
    BLOCK_IF = CMD_IF,
    BLOCK_WAIT = CMD_WAIT,
    BLOCK_SAY = CMD_SAY,
    BLOCK_EVENT_START = CMD_EVENT_CLICK,
    BLOCK_SENSE_TOUCHING_MOUSE = SENSE_TOUCHING_MOUSE,
    BLOCK_SENSE_TOUCHING_EDGE = SENSE_TOUCHING_EDGE,
    BLOCK_OP_ADD = OP_ADD,
    BLOCK_OP_SUB = OP_SUB,
    BLOCK_OP_DIV = OP_DIV
};

struct Block {
    int id;
    BlockType type;
    float x, y;
    float width, height;
    std::vector<std::string> args;
    Block* next = nullptr;
    Block* inner = nullptr;
};

#define BLOCK_WIDTH_DEFAULT 120
#define BLOCK_HEIGHT_DEFAULT 40

struct Color {
    Uint8 r, g, b, a;
};

struct Sprite {
    float x, y;
    float width, height;
    float angle;
    bool visible;
    bool isPenDown;
    int currentCostumeIndex;
    SDL_Texture* texture;
};

struct Stage {
    float x, y;
    float width, height;
    Color border_color;
    Color background_color;
};

struct PaletteItem {
    BlockType type;
    int x, y;
    int width, height;
};

#endif
