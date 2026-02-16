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
        case OP_DIV: return "OP_DIV";
        case CMD_SET_X:    return "SET_X";
        case CMD_SET_Y:    return "SET_Y";
        case CMD_CHANGE_X: return "CHANGE_X";
        case CMD_CHANGE_Y: return "CHANGE_Y";
        case CMD_START:    return "START";
        case CMD_NONE:     return "NONE";
        case CMD_SWITCH_COSTUME: return "SWITCH_COSTUME";
        case CMD_NEXT_COSTUME:   return "NEXT_COSTUME";
        case CMD_SET_SIZE:       return "SET_SIZE";
        case CMD_CHANGE_SIZE:    return "CHANGE_SIZE";
        case CMD_SHOW:           return "SHOW";
        case CMD_HIDE:           return "HIDE";
        case CMD_PLAY_SOUND:     return "PLAY_SOUND";
        case CMD_STOP_ALL_SOUNDS: return "STOP_ALL_SOUNDS";
        case CMD_CHANGE_VOLUME:  return "CHANGE_VOLUME";
        case CMD_SET_VOLUME:     return "SET_VOLUME";
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
    return CMD_MOVE;
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