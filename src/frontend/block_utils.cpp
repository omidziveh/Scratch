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
        case CMD_NONE:     return "---";
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
