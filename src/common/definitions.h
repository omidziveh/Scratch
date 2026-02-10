#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>

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
    BLOCK_MOVE,
    BLOCK_TURN,
    BLOCK_GOTO,
    BLOCK_REPEAT,
    BLOCK_IF,
    BLOCK_WAIT,
    BLOCK_SAY,
    BLOCK_START,
    BLOCK_SET_X,
    BLOCK_SET_Y,
    BLOCK_CHANGE_X,
    BLOCK_CHANGE_Y
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
    bool hasBreakpoint;
    
    Block() : id(0), type(BLOCK_NONE), x(0), y(0), 
              width(BLOCK_WIDTH), height(BLOCK_HEIGHT),
              next(nullptr), inner(nullptr), hasBreakpoint(false) {}
};

struct Sprite {
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
    
    Sprite() : x(0), y(0), width(100), height(100), angle(0),
               texture(nullptr), visible(true), isPenDown(false),
               currentCostumeIndex(0) {}
};

struct Stage {
    int x;
    int y;
    int width;
    int height;
    SDL_Color border_color;
    SDL_Color background_color;
    Stage() : x(STAGE_X), y(STAGE_Y), width(STAGE_WIDTH), height(STAGE_HEIGHT) {
        border_color = {100, 100, 100, 255};
        background_color = {255, 255, 255, 255};
    }
};

struct PaletteItem {
    BlockType type;
    int x;
    int y;
    int width;
    int height;
    Uint8 r;
    Uint8 g;
    Uint8 b;
    const char* label;
    
    PaletteItem() : type(BLOCK_NONE), x(0), y(0), width(0), height(0),
                    r(0), g(0), b(0), label("") {}
};

struct GraphicsContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    GraphicsContext() : window(nullptr), renderer(nullptr) {}
};

void init_graphics(GraphicsContext* ctx);
void shutdown_graphics(GraphicsContext* ctx);
void clear(GraphicsContext* ctx, Uint8 r, Uint8 g, Uint8 b);

#endif // DEFINITIONS_H
