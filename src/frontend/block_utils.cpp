#include "block_utils.h"
#include "../common/globals.h"

bool is_reporter_block(BlockType type) {
    if (type >= OP_ADD && type <= OP_XOR) return true;
    if (type >= SENSE_TOUCHING_MOUSE && type <= SENSE_TIMER) return true;
    return false;
}

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
        case OP_MUL:        return "( * )";
        case OP_DIV:        return "( / )";
        case OP_MOD:        return "( % )";
        case OP_ABS:        return "abs ( 0 )";
        case OP_FLOOR:      return "floor ( 0 )";
        case OP_CEIL:       return "ceiling ( 0 )";
        case OP_SQRT:       return "sqrt ( 0 )";
        case OP_SIN:        return "sin ( 0 )";
        case OP_COS:        return "cos ( 0 )";
        case OP_GT:         return "( > )";
        case OP_LT:         return "( < )";
        case OP_EQ:         return "( = )";
        case OP_AND:        return "( and )";
        case OP_OR:         return "( or )";
        case OP_NOT:        return "not ( )";
        case OP_XOR:        return "xor ( )";
        case OP_STR_LEN:    return "length of ( )";
        case OP_STR_CHAR:   return "letter ( 1 ) of ( )";
        case OP_STR_CONCAT: return "join ( ) ( )";
        case SENSE_MOUSE_DOWN: return "mouse down?";
        case SENSE_MOUSE_X:    return "mouse x";
        case SENSE_MOUSE_Y:    return "mouse y";
        case SENSE_TIMER:      return "timer";
        case SENSE_RESET_TIMER:return "reset timer";
        case CMD_SET_VAR:       return "Set [var] to (0)";
        case CMD_CHANGE_VAR:    return "Change [var] by (1)";
        case CMD_DEFINE_BLOCK: return "Define [name] args:(arg)";
        case CMD_CALL_BLOCK:   return "Call [name] with:(val)";
        case CMD_EVENT_CLICK: return "When sprite clicked";
        case CMD_EVENT_KEY:   return "When [key] key pressed";
        case CMD_GOTO_RANDOM: return "go to random pos";
        case CMD_GOTO_MOUSE: return "go to mouse";
        case CMD_IF_ON_EDGE_BOUNCE: return "if on edge, bounce";
        case SENSE_DISTANCE_TO_MOUSE: return "Dist to mouse";

        default:           return "Unknown";
    }
}

std::string get_header_label(BlockType type) {
    switch (type) {
        case OP_ADD:    return "+";
        case OP_SUB:    return "-";
        case OP_MUL:    return "*";
        case OP_DIV:    return "/";
        case OP_MOD:    return "%";
        case OP_GT:     return ">";
        case OP_LT:     return "<";
        case OP_EQ:     return "=";
        case OP_AND:    return "and";
        case OP_OR:     return "or";
        case OP_XOR:    return "xor";
        case OP_ABS:    return "abs";
        case OP_FLOOR:  return "floor";
        case OP_CEIL:   return "ceil";
        case OP_SQRT:   return "sqrt";
        case OP_SIN:    return "sin";
        case OP_COS:    return "cos";
        case OP_NOT:    return "not";
        case OP_STR_LEN: return "length";
        case OP_STR_CHAR: return "letter";
        case OP_STR_CONCAT: return "join";
        
        case SENSE_MOUSE_DOWN: return "mouse down?";
        case SENSE_MOUSE_X:    return "mouse x";
        case SENSE_MOUSE_Y:    return "mouse y";
        case SENSE_TIMER:      return "timer";
        case CMD_DEFINE_BLOCK: return "Define";
        case CMD_CALL_BLOCK:   return "Call";
        default: break;
    }

    // Default behavior for non-operator blocks
    std::string full = block_get_label(type);
    std::string result;
    bool inArg = false;
    for (char c : full) {
        if (c == '(' || c == '[' || c == '<') {
            inArg = true;
        } else if (c == ')' || c == ']' || c == '>') {
            inArg = false;
        } else if (!inArg) {
            result += c;
        }
    }
    
    if (!result.empty() && result.back() == ' ') result.pop_back();
    return result;
}

std::string get_arg_label(BlockType type, int index) {
    std::string full = block_get_label(type);
    std::string currentArg;
    int argIdx = -1;
    bool inArg = false;

    for (size_t i = 0; i < full.length(); ++i) {
        char c = full[i];
        if (c == '(' || c == '[' || c == '<') {
            argIdx++;
            inArg = true;
            if (argIdx == index) {
                std::string val;
                size_t j = i + 1;
                while (j < full.length() && full[j] != ')' && full[j] != ']' && full[j] != '>') {
                    val += full[j];
                    j++;
            }
                return val;
            }
        } else if (c == ')' || c == ']' || c == '>') {
            inArg = false;
        }
    }
    return "";
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
        case CMD_EVENT_CLICK:
        case CMD_EVENT_KEY:
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
        
        case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV:
        case OP_MOD: case OP_ABS: case OP_FLOOR: case OP_CEIL:
        case OP_SQRT: case OP_SIN: case OP_COS:
        case OP_GT: case OP_LT: case OP_EQ:
        case OP_AND: case OP_OR: case OP_NOT: case OP_XOR:
        case OP_STR_LEN: case OP_STR_CHAR: case OP_STR_CONCAT:
            return COLOR_OPERATOR;

        case SENSE_TOUCHING_MOUSE: case SENSE_TOUCHING_EDGE:
        case SENSE_MOUSE_DOWN: case SENSE_MOUSE_X: case SENSE_MOUSE_Y:
        case SENSE_TIMER: case SENSE_RESET_TIMER:
            return COLOR_SENSING;

        case CMD_SET_VAR:
        case CMD_CHANGE_VAR:
            return COLOR_VARIABLE;

        case CMD_DEFINE_BLOCK:
        case CMD_CALL_BLOCK:
            return COLOR_CUSTOM;
        case CMD_GOTO_RANDOM:
        case CMD_GOTO_MOUSE:
        case CMD_IF_ON_EDGE_BOUNCE:
            return COLOR_MOTION;
        case SENSE_DISTANCE_TO_MOUSE:
            return COLOR_SENSING;

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
        
        case CMD_SET_VAR:     return {"score", "0"};
        case CMD_CHANGE_VAR:  return {"score", "1"};

        case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
        case OP_GT: case OP_LT: case OP_EQ: case OP_AND: case OP_OR: case OP_XOR:
             return {"0", "0"};
             
        case OP_ABS: case OP_FLOOR: case OP_CEIL: case OP_SQRT: case OP_SIN: case OP_COS:
        case OP_NOT: case OP_STR_LEN:
             return {"0"};
             
        case OP_STR_CONCAT: return {"", ""};
        case OP_STR_CHAR: return {"1", ""};

        case CMD_DEFINE_BLOCK: return {"myBlock", "param"};
        case CMD_CALL_BLOCK:   return {"myBlock", "10"};

        case CMD_EVENT_KEY:    return {"space"};

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
        case OP_ABS:
        case OP_FLOOR:
        case OP_CEIL:
        case OP_SQRT:
        case OP_SIN:
        case OP_COS:
        case OP_NOT:
        case OP_STR_LEN:
        case CMD_IF:
        case CMD_PEN_SET_COLOR:
        case CMD_PEN_SET_SIZE:
        case CMD_EVENT_KEY:
            return 1;

        case CMD_GOTO:
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
        case OP_STR_CONCAT:
        case OP_STR_CHAR:
        case CMD_SET_VAR:
        case CMD_CHANGE_VAR:
        case CMD_DEFINE_BLOCK:
        case CMD_CALL_BLOCK:
            return 2;

        case CMD_START:
        case CMD_NEXT_COSTUME:
        case CMD_SHOW:
        case CMD_HIDE:
        case CMD_STOP_ALL_SOUNDS:
        case CMD_NONE:
        case CMD_EVENT_CLICK:
            return 0;
        case CMD_GOTO_RANDOM: return 0;
        case CMD_GOTO_MOUSE: return 0;
        case CMD_IF_ON_EDGE_BOUNCE: return 0;
        case SENSE_DISTANCE_TO_MOUSE: return 0;

        default:
            return 0;
    }
}
SDL_Rect get_arg_box_rect(const Block& block, int arg_index) {
    SDL_Rect rect = {0, 0, 0, 0};

    int count = get_arg_count(block.type);
    if (arg_index < 0 || arg_index >= count)
        return rect;
    
    int argWidth = 50;
    int argHeight = 20;
    int margin = 5;

    if (is_reporter_block(block.type)) {
        rect.w = argWidth;
        rect.h = argHeight;
        rect.y = (int)block.y + (block.height - argHeight) / 2;

        if (count == 1) {
            rect.x = (int)block.x + (int)block.width - margin - argWidth;
        } 
        else if (count == 2) {
            if (arg_index == 0) {
                rect.x = (int)block.x + margin;
            } else {
                rect.x = (int)block.x + (int)block.width - margin - argWidth;
            }
        }
        return rect;
    }

    // Stack blocks: Arguments are OUTSIDE (to the right of) the block
    rect.y = (int)block.y + (BLOCK_HEIGHT - argHeight) / 2;
    rect.x = (int)block.x + (int)block.width + 5;
    
    rect.x += arg_index * (argWidth + margin);

    rect.w = argWidth;
    rect.h = argHeight;

    return rect;
}


int get_total_height(Block* block) {
    if (!block) return 0;
    if (is_reporter_block(block->type)) {
        return 26;
    }

    int h = BLOCK_HEIGHT;
    // h += get_arg_count(block->type) * ARG_ROW_HEIGHT;

    if (block->type == CMD_IF || block->type == CMD_REPEAT || block->type == CMD_DEFINE_BLOCK) {
        if (block->inner) {
            h += 5;
            Block* child = block->inner;
            while (child) {
                h += get_total_height(child);
                child = child->next;
            }
        }
        h += 15;
    }
    return h;
}
