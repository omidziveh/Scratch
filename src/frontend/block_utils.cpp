#include "block_utils.h"
#include "../common/globals.h"

std::string blocktype_to_string(BlockType type) {
    switch (type) {
        case CMD_MOVE:     return "MOVE";
        case CMD_TURN:     return "TURN";
        case CMD_GOTO:     return "GOTO";
        case CMD_SET_X:    return "SET_X";
        case CMD_SET_Y:    return "SET_Y";
        case CMD_CHANGE_X: return "CHANGE_X";
        case CMD_CHANGE_Y: return "CHANGE_Y";
        case CMD_REPEAT:   return "REPEAT";
        case CMD_IF:       return "IF";
        case CMD_WAIT:     return "WAIT";
        case CMD_SAY:      return "SAY";
        case CMD_START:    return "START";
        case CMD_NONE:     return "NONE";
        default:           return "UNKNOWN";
    }
}

BlockType string_to_blocktype(const std::string& str) {
    if (str == "MOVE")     return CMD_MOVE;
    if (str == "TURN")     return CMD_TURN;
    if (str == "GOTO")     return CMD_GOTO;
    if (str == "SET_X")    return CMD_SET_X;
    if (str == "SET_Y")    return CMD_SET_Y;
    if (str == "CHANGE_X") return CMD_CHANGE_X;
    if (str == "CHANGE_Y") return CMD_CHANGE_Y;
    if (str == "REPEAT")   return CMD_REPEAT;
    if (str == "IF")       return CMD_IF;
    if (str == "WAIT")     return CMD_WAIT;
    if (str == "SAY")      return CMD_SAY;
    if (str == "START")    return CMD_START;
    return CMD_NONE;
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
            return COLOR_LOOKS;
        case CMD_START:
            return COLOR_EVENTS;
        default:
            return COLOR_GRAY;
    }
}
