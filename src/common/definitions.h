#pragma once
#include <vector>
#include <string>

enum BlockType { 
    CMD_MOVE, CMD_TURN, CMD_SAY, CMD_WAIT,    // Action
    CMD_IF, CMD_REPEAT,                       // Control
    CMD_EVENT_CLICK                           // Events
};

// The Data Structure (NO METHODS allowed inside)
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
