#include "file_io.h"
#include "memory.h"
#include "../utils/logger.h"
#include <fstream>
#include <sstream>
#include <map>

std::string blocktype_to_string(BlockType type) {
    switch (type) {
        case CMD_MOVE: return "MOVE";
        case CMD_TURN: return "TURN";
        case CMD_GOTO: return "GOTO";
        case CMD_REPEAT: return "REPEAT";
        case CMD_IF: return "IF";
        case CMD_WAIT: return "WAIT";
        case CMD_SAY: return "SAY";
        case CMD_EVENT_CLICK: return "EVENT_CLICK";
        case SENSE_TOUCHING_MOUSE: return "SENSE_MOUSE";
        case SENSE_TOUCHING_EDGE: return "SENSE_EDGE";
        case OP_ADD: return "OP_ADD";
        case OP_SUB: return "OP_SUB";
        case OP_MUL:        return "OP_MUL";
        case OP_DIV:        return "OP_DIV";
        case OP_MOD:        return "OP_MOD";
        case OP_ABS:        return "OP_ABS";
        case OP_FLOOR:      return "OP_FLOOR";
        case OP_CEIL:       return "OP_CEIL";
        case OP_SQRT:       return "OP_SQRT";
        case OP_SIN:        return "OP_SIN";
        case OP_COS:        return "OP_COS";
        case OP_GT:         return "OP_GT";
        case OP_LT:         return "OP_LT";
        case OP_EQ:         return "OP_EQ";
        case OP_AND:        return "OP_AND";
        case OP_OR:         return "OP_OR";
        case OP_NOT:        return "OP_NOT";
        case OP_XOR:        return "OP_XOR";
        case OP_STR_LEN:    return "OP_STR_LEN";
        case OP_STR_CHAR:   return "OP_STR_CHAR";
        case OP_STR_CONCAT: return "OP_STR_CONCAT";
        case SENSE_MOUSE_DOWN: return "SENSE_MOUSE_DOWN";
        case SENSE_MOUSE_X:    return "SENSE_MOUSE_X";
        case SENSE_MOUSE_Y:    return "SENSE_MOUSE_Y";
        case SENSE_TIMER:      return "SENSE_TIMER";
        case SENSE_RESET_TIMER:return "SENSE_RESET_TIMER";
        
        default: return "UNKNOWN";
    }
}

BlockType string_to_blocktype(const std::string& str) {
    if (str == "MOVE") return CMD_MOVE;
    if (str == "TURN") return CMD_TURN;
    if (str == "GOTO") return CMD_GOTO;
    if (str == "REPEAT") return CMD_REPEAT;
    if (str == "IF") return CMD_IF;
    if (str == "WAIT") return CMD_WAIT;
    if (str == "SAY") return CMD_SAY;
    if (str == "EVENT_CLICK") return CMD_EVENT_CLICK;
    if (str == "SENSE_MOUSE") return SENSE_TOUCHING_MOUSE;
    if (str == "SENSE_EDGE") return SENSE_TOUCHING_EDGE;
    if (str == "OP_ADD") return OP_ADD;
    if (str == "OP_SUB") return OP_SUB;
    if (str == "OP_MUL") return OP_MUL;
    if (str == "OP_DIV") return OP_DIV;
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
    if (str == "OP_ADD")    return OP_ADD;
    if (str == "OP_SUB")    return OP_SUB;
    if (str == "OP_MUL")    return OP_MUL;
    if (str == "OP_DIV")    return OP_DIV;
    if (str == "OP_MOD")    return OP_MOD;
    if (str == "OP_ABS")    return OP_ABS;
    if (str == "OP_FLOOR")  return OP_FLOOR;
    if (str == "OP_CEIL")   return OP_CEIL;
    if (str == "OP_SQRT")   return OP_SQRT;
    if (str == "OP_SIN")    return OP_SIN;
    if (str == "OP_COS")    return OP_COS;
    if (str == "OP_GT")     return OP_GT;
    if (str == "OP_LT")     return OP_LT;
    if (str == "OP_EQ")     return OP_EQ;
    if (str == "OP_AND")    return OP_AND;
    if (str == "OP_OR")     return OP_OR;
    if (str == "OP_NOT")    return OP_NOT;
    if (str == "OP_XOR")    return OP_XOR;
    if (str == "OP_STR_LEN")    return OP_STR_LEN;
    if (str == "OP_STR_CHAR")   return OP_STR_CHAR;
    if (str == "OP_STR_CONCAT") return OP_STR_CONCAT;
    if (str == "SENSE_MOUSE_DOWN") return SENSE_MOUSE_DOWN;
    if (str == "SENSE_MOUSE_X")    return SENSE_MOUSE_X;
    if (str == "SENSE_MOUSE_Y")    return SENSE_MOUSE_Y;
    if (str == "SENSE_TIMER")      return SENSE_TIMER;
    if (str == "SENSE_RESET_TIMER")return SENSE_RESET_TIMER;
    return CMD_NONE;
}

void save_block_recursive(std::ofstream& file, Block* b, int parentId, int slot) {
    if (!b) return;

    file << b->id << " "
         << blocktype_to_string(b->type) << " "
         << b->x << " " << b->y << " "
         << b->width << " " << b->height << " "
         << parentId << " " << slot << " "
         << b->args.size();

    for (const auto& arg : b->args) {
        file << " " << arg;
    }
    file << "\n";

    // Slot 0 = Inner, Slot 1 = Next
    save_block_recursive(file, b->inner, b->id, 0);
    save_block_recursive(file, b->next, b->id, 1);
}

bool save_to_file(Block* head, std::string filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        log_error("Cannot open file for writing: " + filename);
        return false;
    }

    save_block_recursive(file, head, -1, 1);

    file.close();
    log_success("Saved to " + filename);
    return true;
}

Block* load_from_file(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        log_error("Cannot open file for reading: " + filename);
        return nullptr;
    }

    std::map<int, Block*> blocks;
    std::vector<std::tuple<int, int, int>> links; // childId, parentId, slot

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);

        int id;
        std::string typeStr;
        float x, y, w, h;
        int parentId, slot, argCount;

        ss >> id >> typeStr >> x >> y >> w >> h >> parentId >> slot >> argCount;

        Block* b = create_block(string_to_blocktype(typeStr));
        b->id = id;
        b->x = x;
        b->y = y;
        b->width = w;
        b->height = h;
        b->args.clear();

        for (int i = 0; i < argCount; i++) {
            std::string arg;
            ss >> arg;
            b->args.push_back(arg);
        }

        blocks[id] = b;
        if (parentId != -1) {
            links.push_back({id, parentId, slot});
        }
    }

    for (auto& link : links) {
        int childId = std::get<0>(link);
        int parentId = std::get<1>(link);
        int slot = std::get<2>(link);

        if (blocks.find(childId) != blocks.end() && blocks.find(parentId) != blocks.end()) {
            Block* child = blocks[childId];
            Block* parent = blocks[parentId];
            
            child->parent = parent;
            if (slot == 0) parent->inner = child;
            else parent->next = child;
        }
    }

    file.close();
    
    if (!blocks.empty()) {
        auto last = blocks.rbegin();
        reset_block_counter(last->first + 1);
    }

    log_success("Loaded from " + filename);
    
    for (auto& pair : blocks) {
        if (pair.second->parent == nullptr) return pair.second;
    }
    return nullptr;
}

void save_sprite(const Sprite& sprite, std::string filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << sprite.x << " " << sprite.y << " " << sprite.angle << " "
         << sprite.isPenDown << " " << sprite.currentCostumeIndex << "\n";
    file.close();
}

Sprite load_sprite(std::string filename) {
    Sprite s;
    std::ifstream file(filename);
    if (!file.is_open()) return s;

    int penDown;
    file >> s.x >> s.y >> s.angle >> penDown >> s.currentCostumeIndex;
    s.isPenDown = (penDown != 0);
    file.close();
    return s;
}