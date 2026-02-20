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
        // === Motion: 1 arg ===
        case CMD_MOVE:
            b->args.push_back("10");
            break;
        case CMD_TURN:
            b->args.push_back("15");
            break;
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
            b->args.push_back("0");
            break;

        // === Motion: 2 args ===
        case CMD_GOTO:
            b->args.push_back("0");
            b->args.push_back("0");
            break;

        // === Control ===
        case CMD_REPEAT:
            b->args.push_back("10");
            break;
        case CMD_WAIT:
            b->args.push_back("1");
            break;

        // === Looks ===
        case CMD_SAY:
            b->args.push_back("Hello!");
            break;
        case CMD_SWITCH_COSTUME:
            b->args.push_back("1");
            break;
        case CMD_SET_SIZE:
            b->args.push_back("100");
            break;
        case CMD_CHANGE_SIZE:
            b->args.push_back("10");
            break;

        // === Sound ===
        case CMD_PLAY_SOUND:
            b->args.push_back("meow");
            break;
        case CMD_CHANGE_VOLUME:
            b->args.push_back("10");
            break;
        case CMD_SET_VOLUME:
            b->args.push_back("100");
            break;

        // === Pen ===
        case CMD_PEN_SET_COLOR:
            b->args.push_back("0");
            break;
        case CMD_PEN_SET_SIZE:
            b->args.push_back("1");
            break;

        // === Events ===
        case CMD_EVENT_KEY:
            b->args.push_back("space");
            break;

        // === Variables ===
        case CMD_SET_VAR:
            b->args.push_back("var");
            b->args.push_back("0");
            break;
        case CMD_CHANGE_VAR:
            b->args.push_back("var");
            b->args.push_back("1");
            break;

        // === Custom Blocks ===
        case CMD_DEFINE_BLOCK:
            b->args.push_back("myBlock");
            b->args.push_back("param");
            break;
        case CMD_CALL_BLOCK:
            b->args.push_back("myBlock");
            b->args.push_back("10");
            break;

        // === Operators: Binary (2 args) ===
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_GT:
        case OP_LT:
        case OP_EQ:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            b->args.push_back("0");
            b->args.push_back("0");
            break;

        // === Operators: Unary (1 arg) ===
        case OP_ABS:
        case OP_FLOOR:
        case OP_CEIL:
        case OP_SQRT:
        case OP_SIN:
        case OP_COS:
        case OP_TAN:
        case OP_ROUND:
        case OP_LOG:
        case OP_LN:
        case OP_NOT:
            b->args.push_back("0");
            break;

        // === Operators: Random (2 args) ===
        case OP_RANDOM:
            b->args.push_back("1");
            b->args.push_back("10");
            break;

        // === String Operators ===
        case OP_STR_LEN:
            b->args.push_back("");
            break;
        case OP_STR_CHAR:
            b->args.push_back("1");
            b->args.push_back("");
            break;
        case OP_STR_CONCAT:
            b->args.push_back("");
            b->args.push_back("");
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
