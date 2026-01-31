#include "memory.h"
#include <iostream>

// Creates and initializes a new block with the given type
// Returns a pointer to the newly allocated block
Block* create_block(BlockType t) {
    Block* newBlock = new Block;
    
    static int blockIdCounter = 1;
    
    newBlock->id = blockIdCounter++;
    newBlock->type = t;
    
    newBlock->x = 0.0f;
    newBlock->y = 0.0f;
    newBlock->width = 100.0f;   // Default width
    newBlock->height = 40.0f;   // Default height
    
    newBlock->next = nullptr;
    newBlock->inner = nullptr;
    
    // HEADS UP:
    // switch(t) {
    //     case CMD_MOVE:
    //         newBlock->args.push_back("10");  // Default: move 10 steps
    //         break;
    //     case CMD_TURN:
    //         newBlock->args.push_back("15");  // Default: turn 15 degrees
    //         break;
    //     case CMD_SAY:
    //         newBlock->args.push_back("Hello!"); // Default message
    //         newBlock->args.push_back("2");      // Default duration
    //         break;
    //     case CMD_WAIT:
    //         newBlock->args.push_back("1");   // Default: wait 1 second
    //         break;
    //     case CMD_REPEAT:
    //         newBlock->args.push_back("10");  // Default: repeat 10 times
    //         break;
    //     default:
    //         break;
    // }
    
    return newBlock;
}


// Recursively deletes a chain of blocks
void delete_chain(Block* b) {
    if (b == nullptr) {
        return;
    }
    
    if (b->inner != nullptr) {
        delete_chain(b->inner);
        b->inner = nullptr;
    }
    Block* nextBlock = b->next;
    b->args.clear();
    
    delete b;
    
    if (nextBlock != nullptr) {
        delete_chain(nextBlock);
    }
}

// Deletes a single block and its inner chain
void delete_block(Block* b) {
    if (b == nullptr) {
        return;
    }

    if (b->inner != nullptr) {
        delete_chain(b->inner);
        b->inner = nullptr;
    }
    b->args.clear();
    
    delete b;
}


// Utility function to count total blocks in a chain
int count_blocks(Block* b) {
    if (b == nullptr) {
        return 0;
    }

    int count = 1;
    if (b->inner != nullptr) {
        count += count_blocks(b->inner);
    }
    if (b->next != nullptr) {
        count += count_blocks(b->next);
    }
    
    return count;
}
