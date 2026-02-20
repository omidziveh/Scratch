#include "file_io.h"
#include "memory.h"
#include "../utils/logger.h"
#include "../frontend/block_utils.h"
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>


std::string blocktype_to_string(BlockType type) {
    switch (type) {
        case CMD_MOVE: return "MOVE";
        case CMD_TURN: return "TURN";
        case CMD_GOTO: return "GOTO";
        case CMD_SET_X: return "SET_X";
        case CMD_SET_Y: return "SET_Y";
        case CMD_CHANGE_X: return "CHANGE_X";
        case CMD_CHANGE_Y: return "CHANGE_Y";
        case CMD_REPEAT: return "REPEAT";
        case CMD_IF: return "IF";
        case CMD_WAIT: return "WAIT";
        case CMD_SAY: return "SAY";
        case CMD_START: return "START";
        case CMD_EVENT_CLICK: return "EVENT_CLICK";
        case CMD_EVENT_KEY: return "EVENT_KEY";
        
        case CMD_SWITCH_COSTUME: return "SWITCH_COSTUME";
        case CMD_NEXT_COSTUME: return "NEXT_COSTUME";
        case CMD_SET_SIZE: return "SET_SIZE";
        case CMD_CHANGE_SIZE: return "CHANGE_SIZE";
        case CMD_SHOW: return "SHOW";
        case CMD_HIDE: return "HIDE";

        case CMD_PLAY_SOUND: return "PLAY_SOUND";
        case CMD_STOP_ALL_SOUNDS: return "STOP_ALL_SOUNDS";
        case CMD_CHANGE_VOLUME: return "CHANGE_VOLUME";
        case CMD_SET_VOLUME: return "SET_VOLUME";

        case SENSE_TOUCHING_MOUSE: return "SENSE_MOUSE";
        case SENSE_TOUCHING_EDGE: return "SENSE_EDGE";
        case SENSE_MOUSE_DOWN: return "SENSE_MOUSE_DOWN";
        case SENSE_MOUSE_X: return "SENSE_MOUSE_X";
        case SENSE_MOUSE_Y: return "SENSE_MOUSE_Y";
        case SENSE_TIMER: return "SENSE_TIMER";
        case SENSE_RESET_TIMER: return "SENSE_RESET_TIMER";

        case OP_ADD: return "OP_ADD";
        case OP_SUB: return "OP_SUB";
        case OP_MUL: return "OP_MUL";
        case OP_DIV: return "OP_DIV";
        case OP_MOD: return "OP_MOD";
        case OP_ABS: return "OP_ABS";
        case OP_FLOOR: return "OP_FLOOR";
        case OP_CEIL: return "OP_CEIL";
        case OP_SQRT: return "OP_SQRT";
        case OP_SIN: return "OP_SIN";
        case OP_COS: return "OP_COS";
        case OP_GT: return "OP_GT";
        case OP_LT: return "OP_LT";
        case OP_EQ: return "OP_EQ";
        case OP_AND: return "OP_AND";
        case OP_OR: return "OP_OR";
        case OP_NOT: return "OP_NOT";
        case OP_XOR: return "OP_XOR";
        case OP_STR_LEN: return "OP_STR_LEN";
        case OP_STR_CHAR: return "OP_STR_CHAR";
        case OP_STR_CONCAT: return "OP_STR_CONCAT";

        case CMD_PEN_DOWN: return "PEN_DOWN";
        case CMD_PEN_UP: return "PEN_UP";
        case CMD_PEN_CLEAR: return "PEN_CLEAR";
        case CMD_PEN_SET_COLOR: return "PEN_SET_COLOR";
        case CMD_PEN_SET_SIZE: return "PEN_SET_SIZE";
        case CMD_PEN_STAMP: return "PEN_STAMP";

        case CMD_SET_VAR: return "SET_VAR";
        case CMD_CHANGE_VAR: return "CHANGE_VAR";

        case CMD_DEFINE_BLOCK: return "DEFINE_BLOCK";
        case CMD_CALL_BLOCK:   return "CALL_BLOCK";
        case CMD_GOTO_RANDOM: return "GOTO_RANDOM";
        case CMD_GOTO_MOUSE: return "GOTO_MOUSE";
        case CMD_IF_ON_EDGE_BOUNCE: return "IF_ON_EDGE_BOUNCE";
        case SENSE_DISTANCE_TO_MOUSE: return "SENSE_DISTANCE_MOUSE";
        case OP_ROUND: return "OP_ROUND";
        case OP_TAN: return "OP_TAN";
        case OP_ASIN: return "OP_ASIN";
        case OP_ACOS: return "OP_ACOS";
        case OP_ATAN: return "OP_ATAN";
        case OP_LN: return "OP_LN";
        case OP_LOG: return "OP_LOG";
        case OP_E_POW: return "OP_E_POW";
        case OP_TEN_POW: return "OP_TEN_POW";
        case OP_RANDOM: return "OP_RANDOM";

        default: return "UNKNOWN";
    }
}

BlockType string_to_blocktype(const std::string& str) {
    if (str == "MOVE") return CMD_MOVE;
    if (str == "TURN") return CMD_TURN;
    if (str == "GOTO") return CMD_GOTO;
    if (str == "SET_X") return CMD_SET_X;
    if (str == "SET_Y") return CMD_SET_Y;
    if (str == "CHANGE_X") return CMD_CHANGE_X;
    if (str == "CHANGE_Y") return CMD_CHANGE_Y;
    if (str == "REPEAT") return CMD_REPEAT;
    if (str == "IF") return CMD_IF;
    if (str == "WAIT") return CMD_WAIT;
    if (str == "SAY") return CMD_SAY;
    if (str == "START") return CMD_START;
    if (str == "EVENT_CLICK") return CMD_EVENT_CLICK;
    if (str == "EVENT_KEY") return CMD_EVENT_KEY;
    
    if (str == "SWITCH_COSTUME") return CMD_SWITCH_COSTUME;
    if (str == "NEXT_COSTUME") return CMD_NEXT_COSTUME;
    if (str == "SET_SIZE") return CMD_SET_SIZE;
    if (str == "CHANGE_SIZE") return CMD_CHANGE_SIZE;
    if (str == "SHOW") return CMD_SHOW;
    if (str == "HIDE") return CMD_HIDE;

    if (str == "PLAY_SOUND") return CMD_PLAY_SOUND;
    if (str == "STOP_ALL_SOUNDS") return CMD_STOP_ALL_SOUNDS;
    if (str == "CHANGE_VOLUME") return CMD_CHANGE_VOLUME;
    if (str == "SET_VOLUME") return CMD_SET_VOLUME;

    if (str == "SENSE_MOUSE") return SENSE_TOUCHING_MOUSE;
    if (str == "SENSE_EDGE") return SENSE_TOUCHING_EDGE;
    if (str == "SENSE_MOUSE_DOWN") return SENSE_MOUSE_DOWN;
    if (str == "SENSE_MOUSE_X") return SENSE_MOUSE_X;
    if (str == "SENSE_MOUSE_Y") return SENSE_MOUSE_Y;
    if (str == "SENSE_TIMER") return SENSE_TIMER;
    if (str == "SENSE_RESET_TIMER") return SENSE_RESET_TIMER;

    if (str == "OP_ADD") return OP_ADD;
    if (str == "OP_SUB") return OP_SUB;
    if (str == "OP_MUL") return OP_MUL;
    if (str == "OP_DIV") return OP_DIV;
    if (str == "OP_MOD") return OP_MOD;
    if (str == "OP_ABS") return OP_ABS;
    if (str == "OP_FLOOR") return OP_FLOOR;
    if (str == "OP_CEIL") return OP_CEIL;
    if (str == "OP_SQRT") return OP_SQRT;
    if (str == "OP_SIN") return OP_SIN;
    if (str == "OP_COS") return OP_COS;
    if (str == "OP_GT") return OP_GT;
    if (str == "OP_LT") return OP_LT;
    if (str == "OP_EQ") return OP_EQ;
    if (str == "OP_AND") return OP_AND;
    if (str == "OP_OR") return OP_OR;
    if (str == "OP_NOT") return OP_NOT;
    if (str == "OP_XOR") return OP_XOR;
    if (str == "OP_STR_LEN") return OP_STR_LEN;
    if (str == "OP_STR_CHAR") return OP_STR_CHAR;
    if (str == "OP_STR_CONCAT") return OP_STR_CONCAT;

    if (str == "PEN_DOWN") return CMD_PEN_DOWN;
    if (str == "PEN_UP") return CMD_PEN_UP;
    if (str == "PEN_CLEAR") return CMD_PEN_CLEAR;
    if (str == "PEN_SET_COLOR") return CMD_PEN_SET_COLOR;
    if (str == "PEN_SET_SIZE") return CMD_PEN_SET_SIZE;
    if (str == "PEN_STAMP") return CMD_PEN_STAMP;

    if (str == "SET_VAR") return CMD_SET_VAR;
    if (str == "CHANGE_VAR") return CMD_CHANGE_VAR;

    if (str == "DEFINE_BLOCK") return CMD_DEFINE_BLOCK;
    if (str == "CALL_BLOCK")   return CMD_CALL_BLOCK;
    if (str == "GOTO_RANDOM") return CMD_GOTO_RANDOM;
    if (str == "GOTO_MOUSE") return CMD_GOTO_MOUSE;
    if (str == "IF_ON_EDGE_BOUNCE") return CMD_IF_ON_EDGE_BOUNCE;
    if (str == "SENSE_DISTANCE_MOUSE") return SENSE_DISTANCE_TO_MOUSE;
    if (str == "OP_ROUND") return OP_ROUND;
    if (str == "OP_TAN") return OP_TAN;
    if (str == "OP_ASIN") return OP_ASIN;
    if (str == "OP_ACOS") return OP_ACOS;
    if (str == "OP_ATAN") return OP_ATAN;
    if (str == "OP_LN") return OP_LN;
    if (str == "OP_LOG") return OP_LOG;
    if (str == "OP_E_POW") return OP_E_POW;
    if (str == "OP_TEN_POW") return OP_TEN_POW;
    if (str == "OP_RANDOM") return OP_RANDOM;

    return CMD_NONE;
}

static void save_block_recursive(std::ofstream& file, Block* b, int parentId, int slot) {
    if (!b) return;

    file << "BLOCK "
         << b->id << " "
         << blocktype_to_string(b->type) << " "
         << b->x << " " << b->y << " "
         << b->width << " " << b->height << " "
         << parentId << " " << slot << " "
         << b->args.size();

    for (const auto& arg : b->args) {
        file << " " << arg;
    }
    file << "\n";

    for (size_t i = 0; i < b->argBlocks.size(); i++) {
        if (b->argBlocks[i]) {
            file << "ARG_LINK " << b->id << " " << i << " " << b->argBlocks[i]->id << "\n";
        }
    }

    save_block_recursive(file, b->inner, b->id, 0);
    save_block_recursive(file, b->next, b->id, 1);
}

bool save_project(const std::string& filename, const std::list<Block>& blocks, const Sprite& sprite) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        log_error("Cannot open file for writing: " + filename);
        return false;
    }

    // 1. Save Sprite Data
    file << "SPRITE " 
         << sprite.x << " " 
         << sprite.y << " " 
         << sprite.angle << " "
         << sprite.visible << " "
         << sprite.scale << " "
         << sprite.volume << " "
         << sprite.isPenDown << " "
         << (int)sprite.penR << " "
         << (int)sprite.penG << " "
         << (int)sprite.penB << " "
         << sprite.penSize << " "
         << sprite.currentCostumeIndex << "\n";

    // 2. Save Variables
    for (const auto& var : sprite.variables) {
        file << "VAR " << var.name << " " << var.value << "\n";
    }

    // 3. Save Blocks
    for (const auto& b : blocks) {
        int parentId = -1;
        int slot = -1; // 0=inner, 1=next, -1=root/arg

        if (b.parent) {
            parentId = b.parent->id;
            if (b.parent->next == &b) slot = 1;
            else if (b.parent->inner == &b) slot = 0;
            else {
                // It's an argument block. 
                // We don't save this connection via PARENT line, 
                // we rely on ARG_LINK from the host block to catch it.
                parentId = -1; 
            }
        }

        file << "BLOCK "
             << b.id << " "
             << blocktype_to_string(b.type) << " "
             << b.x << " " << b.y << " "
             << b.width << " " << b.height << " "
             << parentId << " " << slot << " "
             << b.args.size();

        for (const auto& arg : b.args) {
            file << " " << arg.length() << " " << arg;
        }
        file << "\n";

        // Save Argument Block Links
        for (size_t i = 0; i < b.argBlocks.size(); i++) {
            if (b.argBlocks[i]) {
                file << "ARG_LINK " << b.id << " " << i << " " << b.argBlocks[i]->id << "\n";
            }
        }
    }

    file.close();
    log_success("Saved project to " + filename);
    return true;
}

bool load_project(const std::string& filename, std::list<Block>& blocks, Sprite& sprite, int& next_block_id) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        log_error("Cannot open file for reading: " + filename);
        return false;
    }

    blocks.clear();
    sprite.variables.clear();
    
    std::map<int, Block*> idToPointer;
    std::vector<std::tuple<int, int, int>> pendingLinks;
    std::vector<std::tuple<int, int, int>> pendingArgLinks;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "SPRITE") {
            int r, g, b;
            ss >> sprite.x >> sprite.y >> sprite.angle >> sprite.visible 
               >> sprite.scale >> sprite.volume >> sprite.isPenDown
               >> r >> g >> b >> sprite.penSize >> sprite.currentCostumeIndex;
            
            sprite.penR = (Uint8)r;
            sprite.penG = (Uint8)g;
            sprite.penB = (Uint8)b;
        } 
        else if (type == "VAR") {
            std::string name, value;
            ss >> name >> value;
            sprite.variables.push_back(Variable(name, value));
        }
        else if (type == "BLOCK") {
            int id;
            std::string typeStr;
            float x, y, w, h;
            int parentId, slot, argCount;

            ss >> id >> typeStr >> x >> y >> w >> h >> parentId >> slot >> argCount;

            blocks.push_back(Block()); 
            Block* b = &blocks.back();
            
            b->id = id;
            b->type = string_to_blocktype(typeStr);
            b->x = x;
            b->y = y;
            b->width = w;
            b->height = h;
            b->color = block_get_color(b->type);
            
            int expectedArgs = get_arg_count(b->type);
            if (expectedArgs > 0) b->argBlocks.resize(expectedArgs, nullptr);
            
            for (int i = 0; i < argCount; i++) {
                size_t len;
                ss >> len;
                ss.ignore(1); 
                
                std::string arg(len, ' ');
                ss.read(&arg[0], len);
                b->args.push_back(arg);
            }

            idToPointer[id] = b;
            if (parentId != -1) {
                pendingLinks.push_back({id, parentId, slot});
            }
        }
        else if (type == "ARG_LINK") {
            int hostId, slot, childId;
            ss >> hostId >> slot >> childId;
            pendingArgLinks.push_back({hostId, slot, childId});
        }
    }

    for (auto& link : pendingLinks) {
        int childId = std::get<0>(link);
        int parentId = std::get<1>(link);
        int slot = std::get<2>(link);

        if (idToPointer.count(childId) && idToPointer.count(parentId)) {
            Block* child = idToPointer[childId];
            Block* parent = idToPointer[parentId];
            
            child->parent = parent;
            if (slot == 0) parent->inner = child;
            else if (slot == 1) parent->next = child;
        }
    }

    for (auto& link : pendingArgLinks) {
        int hostId = std::get<0>(link);
        int slot = std::get<1>(link);
        int childId = std::get<2>(link);

        if (idToPointer.count(hostId) && idToPointer.count(childId)) {
            Block* host = idToPointer[hostId];
            Block* child = idToPointer[childId];
            
            if (slot >= (int)host->argBlocks.size()) {
                host->argBlocks.resize(slot + 1, nullptr);
            }
            
            host->argBlocks[slot] = child;
            child->parent = host;
            child->is_snapped = true;
        }
    }

    file.close();
    
    if (!idToPointer.empty()) {
        auto last = idToPointer.rbegin();
        next_block_id = last->first + 1;
        reset_block_counter(next_block_id);
    } else {
        next_block_id = 1;
    }

    log_success("Loaded project from " + filename);
    return true;
}
