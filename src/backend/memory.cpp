#include "memory.h"
#include "../utils/logger.h"

static int blockIdCounter = 1;

void reset_block_counter(int newValue) {
    blockIdCounter = newValue;
}

int get_block_counter() {
    return blockIdCounter;
}

Block* create_block(BlockType t) {
    Block* b = new Block;
    b->id = blockIdCounter++;
    b->type = t;
    b->x = 0.0f;
    b->y = 0.0f;
    b->width = 100.0f;
    b->height = 40.0f;
    b->next = nullptr;
    b->inner = nullptr;
    b->is_running = false;
    b->glow_start_time = 0;
    switch (t) {
        case CMD_MOVE:
            b->args.push_back("10");
            break;
        case CMD_TURN:
            b->args.push_back("15");
            break;
        case CMD_SAY:
            b->args.push_back("Hello!");
            break;
        case CMD_WAIT:
            b->args.push_back("1");
            break;
        case CMD_REPEAT:
            b->args.push_back("10");
            break;
        default:
            break;
    }

    return b;
}

void delete_block(Block* b) {
    if (!b) return;
    delete b;
}

void delete_chain(Block* b) {
    if (!b) return;

    delete_chain(b->inner);
    delete_chain(b->next);

    delete b;
}

int count_blocks(Block* b) {
    if (!b) return 0;
    return 1 + count_blocks(b->inner) + count_blocks(b->next);
}
