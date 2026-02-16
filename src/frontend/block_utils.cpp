#include "block_utils.h"
#include "../common/globals.h"

std::string block_get_label(BlockType type) {
    switch (type) {
        case CMD_MOVE:     return "Move (10) steps";
        case CMD_TURN:     return "Turn (15) degrees";
        case CMD_GOTO:     return "Go to x:(0) y:(0)";
        case CMD_SET_X:    return "Set x to (0)";
        case CMD_SET_Y:    return "Set y to (0)";
        case CMD_CHANGE_X: return "Change x by (10)";
        case CMD_CHANGE_Y: return "Change y by (10)";
        case CMD_REPEAT:   return "Repeat (10)";
        case CMD_IF:       return "If <> then";
        case CMD_WAIT:     return "Wait (1) secs";
        case CMD_SAY:      return "Say [Hello!]";
        case CMD_START:    return "When START clicked";
        case CMD_SWITCH_COSTUME: return "Switch costume to (1)";
        case CMD_NEXT_COSTUME:   return "Next costume";
        case CMD_SET_SIZE:       return "Set size to (100)%";
        case CMD_CHANGE_SIZE:    return "Change size by (10)";
        case CMD_SHOW:           return "Show";
        case CMD_HIDE:           return "Hide";
        case CMD_NONE:             return "---";
        case CMD_PLAY_SOUND:       return "Play sound";
        case CMD_STOP_ALL_SOUNDS:  return "Stop all sounds";
        case CMD_CHANGE_VOLUME:    return "Change volume by";
        case CMD_SET_VOLUME:       return "Set volume to";
        case CMD_PEN_DOWN: return "pen down";
        case CMD_PEN_UP: return "pen up";
        case CMD_PEN_CLEAR: return "clear pen";
        case CMD_PEN_SET_COLOR: return "set pen color";
        case CMD_PEN_SET_SIZE: return "set pen size";
        case CMD_PEN_STAMP: return "stamp";
        case OP_ADD:        return "( + )";
        case OP_SUB:        return "( - )";
        case OP_DIV:        return "( / )"; 

        default:           return "Unknown";
    }
}

SDL_Color block_get_color(BlockType type) {
    switch (type) {
        case CMD_MOVE:
        case CMD_TURN:
        case CMD_GOTO:
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
            return COLOR_MOTION;
        case CMD_REPEAT:
        case CMD_IF:
        case CMD_WAIT:
            return COLOR_CONTROL;
        case CMD_SAY:
        case CMD_SWITCH_COSTUME:
        case CMD_NEXT_COSTUME:
        case CMD_SET_SIZE:
        case CMD_CHANGE_SIZE:
        case CMD_SHOW:
        case CMD_HIDE:
            return COLOR_LOOKS;
        case CMD_START:
            return COLOR_EVENTS;
        case CMD_PLAY_SOUND:
        case CMD_STOP_ALL_SOUNDS:
        case CMD_CHANGE_VOLUME:
        case CMD_SET_VOLUME:
            return COLOR_SOUND;

        case CMD_PEN_CLEAR:
        case CMD_PEN_DOWN:
        case CMD_PEN_SET_COLOR:
        case CMD_PEN_SET_SIZE:
        case CMD_PEN_STAMP:
        case CMD_PEN_UP:
            return COLOR_PEN;
        
        case OP_ADD:
        case OP_SUB:
        case OP_DIV:
            return COLOR_OPERATOR;
        
        default:
            return COLOR_GRAY;
    }
}

std::vector<std::string> get_default_args(BlockType type) {
    switch (type) {
        case CMD_MOVE:     return {"10"};
        case CMD_TURN:     return {"15"};
        case CMD_GOTO:     return {"0", "0"};
        case CMD_SET_X:    return {"0"};
        case CMD_SET_Y:    return {"0"};
        case CMD_CHANGE_X: return {"10"};
        case CMD_CHANGE_Y: return {"10"};
        case CMD_REPEAT:   return {"10"};
        case CMD_WAIT:     return {"1"};
        case CMD_SAY:      return {"Hello!"};
        case CMD_SWITCH_COSTUME: return {"1"};
        case CMD_SET_SIZE:       return {"100"};
        case CMD_CHANGE_SIZE:    return {"10"};
        default:           return {};
    }
}
int get_arg_count(BlockType type) {
    switch (type) {
        case CMD_MOVE:
        case CMD_TURN:
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
        case CMD_REPEAT:
        case CMD_WAIT:
        case CMD_SAY:
        case CMD_SWITCH_COSTUME:
        case CMD_SET_SIZE:
        case CMD_CHANGE_SIZE:
        case CMD_PLAY_SOUND:
        case CMD_CHANGE_VOLUME:
        case CMD_SET_VOLUME:
            return 1;

        case CMD_GOTO:
        case OP_DIV:
        case OP_ADD:
        case OP_SUB:
            return 2;

        case CMD_START:
        case CMD_NEXT_COSTUME:
        case CMD_SHOW:
        case CMD_HIDE:
        case CMD_STOP_ALL_SOUNDS:
        case CMD_IF:
        case CMD_NONE:
            return 0;

        default:
            return 0;
    }
}

SDL_Rect get_arg_box_rect(const Block& block, int arg_index) {
    SDL_Rect rect = {0, 0, 0, 0};

    int count = get_arg_count(block.type);
    if (arg_index < 0 || arg_index >= count)
        return rect;

    const int ARG_BOX_WIDTH  = 40;
    const int ARG_BOX_HEIGHT = 16;
    const int ARG_PADDING_X  = 30;
    const int ARG_PADDING_Y  = 7;
    const int ARG_SPACING    = 6;

    rect.x = (int)block.x + ARG_PADDING_X + arg_index * (ARG_BOX_WIDTH + ARG_SPACING);
    rect.y = (int)block.y + ARG_PADDING_Y;
    rect.w = ARG_BOX_WIDTH;
    rect.h = ARG_BOX_HEIGHT;
    int max_right = (int)(block.x + block.width) - 4;
    if (rect.x + rect.w > max_right) {
        rect.w = max_right - rect.x;
        if (rect.w < 8) rect.w = 0;
    }

    return rect;
}

int get_total_height(Block* block) {
    if (!block) return 0;
    
    int h = BLOCK_HEIGHT;

    if (block->type == CMD_IF || block-> type == CMD_REPEAT) {
        if (block->inner) {
            Block* child = block->inner;
            h += get_total_height(child);
            child = child->next;
        }
        h += 10;
    }
    return h;
}
