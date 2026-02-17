#include "memory.h"
#include "../utils/logger.h"
#include "../frontend/block_utils.h"
#include <algorithm>

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
    
    std::string label = block_get_label(t);
    float calculated_width = (float)(label.length() * 8 + 20);
    
    b->width = std::max(100.0f, calculated_width);
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
        case CMD_GOTO:
            b->args.push_back("0");
            b->args.push_back("0");
            break;
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
             b->args.push_back("0");
            break;
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            b->args.push_back("0");
            b->args.push_back("0");
            break;
        case CMD_SET_VAR:
            b->args.push_back("var");
            b->args.push_back("0");
            break;
        case CMD_CHANGE_VAR:
            b->args.push_back("var");
            b->args.push_back("1");
            break;
        default:
            break;
    }

    b->argBlocks.resize(get_arg_count(t), nullptr);

    return b;
}


void delete_block(Block* b) {
    if (!b) return;
    delete b;
}

void delete_chain(Block* b) {
    if (!b) return;

    for (auto* sub : b->argBlocks) {
        delete_chain(sub);
    }
    b->argBlocks.clear();

    delete_chain(b->inner);
    delete_chain(b->next);

    delete b;
}

int count_blocks(Block* b) {
    if (!b) return 0;
    return 1 + count_blocks(b->inner) + count_blocks(b->next);
}
void safe_delete_chain(Block* b, Block* parent_block) {
    if (!b) return;

    if (parent_block) {
        if (parent_block->next == b) {
            parent_block->next = nullptr;
        }
        if (parent_block->inner == b) {
            parent_block->inner = nullptr;
        }
    }

    delete_chain(b);
}
