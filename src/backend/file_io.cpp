#include "file_io.h"
#include "memory.h"
#include "../utils/logger.h"
#include <fstream>
#include <sstream>

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
        case CMD_PEN_DOWN: return "PEN_DOWN";
        case CMD_PEN_UP: return "PEN_UP";
        case CMD_PEN_CLEAR: return "PEN_CLEAR";
        case CMD_PEN_SET_COLOR: return "PEN_SET_COLOR";
        case CMD_PEN_SET_SIZE: return "PEN_SET_SIZE";
        case CMD_PEN_STAMP: return "PEN_STAMP";

    }
    return "UNKNOWN";
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
    if (str == "PEN_DOWN") return CMD_PEN_DOWN;
    if (str == "PEN_UP") return CMD_PEN_UP;
    if (str == "PEN_CLEAR") return CMD_PEN_CLEAR;
    if (str == "PEN_SET_COLOR") return CMD_PEN_SET_COLOR;
    if (str == "PEN_SET_SIZE") return CMD_PEN_SET_SIZE;
    if (str == "PEN_STAMP") return CMD_PEN_STAMP;

    return CMD_MOVE;
}

bool save_to_file(Block* head, std::string filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        log_error("Cannot open file for writing: " + filename);
        return false;
    }

    Block* current = head;
    while (current) {
        file << current->id << " "
             << blocktype_to_string(current->type) << " "
             << current->x << " " << current->y << " "
             << current->width << " " << current->height << " "
             << current->args.size();

        for (const auto& arg : current->args) {
            file << " " << arg;
        }
        file << "\n";

        current = current->next;
    }

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

    Block* head = nullptr;
    Block* tail = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        int id;
        std::string typeStr;
        float x, y, w, h;
        int argCount;

        ss >> id >> typeStr >> x >> y >> w >> h >> argCount;

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

        if (!head) {
            head = b;
            tail = b;
        } else {
            tail->next = b;
            tail = b;
        }
    }

    file.close();
    log_success("Loaded from " + filename);
    return head;
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
