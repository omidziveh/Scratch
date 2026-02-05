#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <string>

#define BLOCK_WIDTH 120
#define BLOCK_HEIGHT 40

#define BLOCK_MOVE 0
#define BLOCK_TURN 1
#define BLOCK_GOTO 2
#define BLOCK_REPEAT 3
#define BLOCK_IF 4
#define BLOCK_WAIT 5

enum BlockType {
    CMD_MOVE, CMD_TURN, CMD_SAY, CMD_WAIT,    // Action
    CMD_IF, CMD_REPEAT,                       // Control
    CMD_EVENT_CLICK                           // Events
};

// The Data Structure
struct Block {
    int id;
    BlockType type;
    
    // Visual Data (Frontend modifies these)
    float x, y;
    float width, height;
    
    // Logic Data (Backend reads these)
    // Example: "Move 10" -> args[0] = "10"
    std::vector<std::string> args; 
    
    // Connectivity
    Block* next = nullptr;   // The block attached below
    Block* inner = nullptr;  // The block inside (for IF/REPEAT)
};

struct Sprite {
    float x = 0;
    float y = 0;
    float angle = 0;
    bool isPenDown = false;
    std::vector<std::string> costumes; // Paths to images
    int currentCostumeIndex = 0;
};

namespace BlockCoding {
    struct Block {
        int type;
        int x, y;
        Block* next;
    };
}

#endif
