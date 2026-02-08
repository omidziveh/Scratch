#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

const int STAGE_X = 800;
const int STAGE_Y = 50;
const int STAGE_WIDTH = 380;
const int STAGE_HEIGHT = 300;
const int STAGE_BORDER_SIZE = 3; 
const int BLOCK_WIDTH = 120;
const int BLOCK_HEIGHT = 50;

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
    BLOCK_MOVE = CMD_MOVE,
    BLOCK_TURN = CMD_TURN,
    BLOCK_GOTO = CMD_GOTO,
    BLOCK_REPEAT = CMD_REPEAT,
    BLOCK_IF = CMD_IF,
    BLOCK_WAIT = CMD_WAIT,
    BLOCK_SAY = CMD_SAY,
    BLOCK_EVENT_START = CMD_EVENT_CLICK
};

struct Block {
    int id;
    BlockType type;
    float x;
    float y;
    float width;
    float height;
    Block* next;
    Block* inner;
    std::vector<std::string> args;
};

#define BLOCK_WIDTH 120
#define BLOCK_HEIGHT 40

enum BlockType {
    CMD_MOVE,
    CMD_TURN,
    CMD_GOTO,
    CMD_REPEAT,
    CMD_IF,
    CMD_WAIT,
    CMD_SAY,
    CMD_EVENT_CLICK
};

struct Block {
    int id;
    BlockType type;

    float x, y;
    float width, height;

    std::vector<std::string> args;

    Block* next = nullptr;
    Block* inner = nullptr;

    bool hasBreakpoint = false;
};

struct Sprite {
<<<<<<< HEAD
    float x;
    float y;
    float width;
    float height;
    double angle;
    SDL_Texture* texture;
    int visible;
    bool isPenDown;
    int currentCostumeIndex;
    std::vector<std::string> costumes;
=======
    float x = 0;
    float y = 0;
    float width = 100;
    float height = 100;
    SDL_Texture* texture = nullptr;
    int visible = true;
    float angle = 0;
    bool isPenDown = false;
    std::vector<std::string> costumes;
    int currentCostumeIndex = 0;
>>>>>>> 265a5b0407adbbe5c738e96251f11c7ede8ea6a0
};

struct Stage {
    int x;
    int y;
    int width;
    int height;
    SDL_Color border_color;
    SDL_Color background_color;
};

<<<<<<< HEAD
struct PaletteItem {
    BlockType type;
    int x;
    int y;
    int width;
    int height;
};

#endif
=======
#endif
>>>>>>> 265a5b0407adbbe5c738e96251f11c7ede8ea6a0
