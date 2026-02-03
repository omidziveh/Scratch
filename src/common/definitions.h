#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>

#define BLOCK_WIDTH 120
#define BLOCK_HEIGHT 40

#define BLOCK_MOVE 0
#define BLOCK_TURN 1
#define BLOCK_GOTO 2
#define BLOCK_REPEAT 3
#define BLOCK_IF 4
#define BLOCK_WAIT 5

namespace BlockCoding {
    struct Block {
        int type;
        int x, y;
        Block* next;
    };
}

#endif
