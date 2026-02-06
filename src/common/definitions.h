#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <string>

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
};

struct Sprite {
    float x = 0;
    float y = 0;
    float angle = 0;
    bool isPenDown = false;
    std::vector<std::string> costumes;
    int currentCostumeIndex = 0;
};

#endif