#ifndef DEFINITIONS_H
#define DEFINITIONS_H

namespace BlockCoding {
    enum BlockType {
        BLOCK_MOVE = 0,
        BLOCK_TURN,
        BLOCK_GOTO,
        BLOCK_REPEAT,
        BLOCK_IF,
        BLOCK_WAIT
    };
    
    struct Block {
        BlockType type;
        int x;
        int y;
        Block* next; 
    };
    
    struct Sprite {
        int x;
        int y;
        int angle;
        bool visible;
    };
}

#endif
