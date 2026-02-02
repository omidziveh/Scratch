#include "file_io.h"
#include "memory.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

int MIN_BLOCK_DATA_COUNT = 9;
int MIN_SPRITE_DATA_COUNT = 5;

// Convert BlockType enum to string
std::string blocktype_to_string(BlockType type) {
    switch(type) {
        case CMD_MOVE:        
            return "MOVE";
        case CMD_TURN:        
            return "TURN";
        case CMD_SAY:         
            return "SAY";
        case CMD_WAIT:        
            return "WAIT";
        case CMD_IF:          
            return "IF";
        case CMD_REPEAT:      
            return "REPEAT";
        case CMD_EVENT_CLICK: 
            return "EVENT_CLICK";
        default:              
            return "UNKNOWN";
    }
}

// Convert string to BlockType enum
BlockType string_to_blocktype(const std::string& str) {
    if (str == "MOVE")        
        return CMD_MOVE;
    if (str == "TURN")        
        return CMD_TURN;
    if (str == "SAY")         
        return CMD_SAY;
    if (str == "WAIT")        
        return CMD_WAIT;
    if (str == "IF")          
        return CMD_IF;
    if (str == "REPEAT")      
        return CMD_REPEAT;
    if (str == "EVENT_CLICK") 
        return CMD_EVENT_CLICK;
    return CMD_MOVE;
}

// Save sprite data
bool save_to_file(Block* head, std::string filename) {
    // Add .txt extension if not present
    // if (filename.find(".txt") == std::string::npos && 
    //     filename.find(".csv") == std::string::npos) {
    //     filename += ".txt";
    // }
    log_info("Starting save proccess for " + filename);
    
    std::ofstream outFile("saves/" + filename);
    
    if (!outFile.is_open()) {
        log_error("Could not open file for writing: " + filename);
        return false;
    }
    
    log_debug("File opened successfully");

    // Header
    outFile << "# Scratch Project Save File" << std::endl;
    outFile << "# Format: ID,TYPE,X,Y,WIDTH,HEIGHT,NEXT_ID,INNER_ID,ARG_COUNT,ARG1,ARG2,..." << std::endl;
    outFile << "# --------------------------------------------------" << std::endl;
    
    // Body
    Block* current = head;
    int blockCount = 0;
    while (current != nullptr) {
        blockCount++;
        log_block_info(current, "  Saving: ");
        outFile << current->id << ","
                << blocktype_to_string(current->type) << ","
                << current->x << ","
                << current->y << ","
                << current->width << ","
                << current->height << ",";
        if (current->next != nullptr) {
            outFile << current->next->id;
        } else {
            outFile << "-1";
        }
        outFile << ",";
        
        if (current->inner != nullptr) {
            outFile << current->inner->id;
        } else {
            outFile << "-1";
        }
        outFile << ",";
        outFile << current->args.size();
        for (const auto& arg : current->args) {
            outFile << "," << arg;
        }
        
        outFile << '\n';
        
        if (current->inner != nullptr) {
            Block* innerCurrent = current->inner;
            while (innerCurrent != nullptr) {
                outFile << innerCurrent->id << ","
                        << blocktype_to_string(innerCurrent->type) << ","
                        << innerCurrent->x << ","
                        << innerCurrent->y << ","
                        << innerCurrent->width << ","
                        << innerCurrent->height << ",";
                
                if (innerCurrent->next != nullptr) {
                    outFile << innerCurrent->next->id;
                } else {
                    outFile << "-1";
                }
                outFile << ",";
                
                if (innerCurrent->inner != nullptr) {
                    outFile << innerCurrent->inner->id;
                } else {
                    outFile << "-1";
                }
                outFile << ",";
                
                outFile << innerCurrent->args.size();
                for (const auto& arg : innerCurrent->args) {
                    outFile << "," << arg;
                }
                
                outFile << '\n';
                innerCurrent = innerCurrent->next;
            }
        }
        
        current = current->next;
    }
    
    outFile.close();
    log_success("Project saved successfully: saves/" + filename + " (" + std::to_string(blockCount) + "blocks)");
    return true;
}

// Load a block chain from a file
Block* load_from_file(std::string filename) {
    // Add .txt extension if not present
    // if (filename.find(".txt") == std::string::npos && 
    //     filename.find(".csv") == std::string::npos) {
    //     filename += ".txt";
    // }

    log_info("Starting load proccess for: " + filename);
    
    std::ifstream inFile("saves/" + filename);
    
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
        return nullptr;
    }
    
    std::map<int, Block*> blockMap;
    std::map<int, int> nextIdMap;
    std::map<int, int> innerIdMap;
    
    Block* head = nullptr;
    std::string line;
    
    while (std::getline(inFile, line)) {
        // Header
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        if ((int)tokens.size() < MIN_BLOCK_DATA_COUNT) {
            continue;
        }
        
        int id = std::stoi(tokens[0]);
        BlockType type = string_to_blocktype(tokens[1]);
        float x = std::stof(tokens[2]);
        float y = std::stof(tokens[3]);
        float width = std::stof(tokens[4]);
        float height = std::stof(tokens[5]);
        int nextId = std::stoi(tokens[6]);
        int innerId = std::stoi(tokens[7]);
        int argCount = std::stoi(tokens[8]);
        
        Block* block = create_block(type);
        block->id = id;
        block->x = x;
        block->y = y;
        block->width = width;
        block->height = height;
        
        block->args.clear(); // Heads up -> potential removal
        for (int i = 0; i < argCount && (MIN_BLOCK_DATA_COUNT + i) < (int)tokens.size(); i++) {
            block->args.push_back(tokens[MIN_BLOCK_DATA_COUNT + i]);
        }
        
        blockMap[id] = block;
        nextIdMap[id] = nextId;
        innerIdMap[id] = innerId;
        
        if (head == nullptr) {
            head = block;
        }
    }
    
    inFile.close();
    
    for (auto& pair : blockMap) {
        int id = pair.first;
        Block* block = pair.second;
        
        if (nextIdMap[id] != -1 && blockMap.find(nextIdMap[id]) != blockMap.end()) {
            block->next = blockMap[nextIdMap[id]];
        }
        
        if (innerIdMap[id] != -1 && blockMap.find(innerIdMap[id]) != blockMap.end()) {
            block->inner = blockMap[innerIdMap[id]];
        }
    }
    
    log_success("Project loaded successfully from: saves/" + filename);
    return head;
}

// Save sprite data
void save_sprite(const Sprite& sprite, std::string filename) {
    std::ofstream outFile("saves/" + filename + "_sprite.txt");
    
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not save sprite data" << std::endl;
        return;
    }
    
    outFile << "# Sprite Data" << std::endl;
    outFile << sprite.x << "," 
            << sprite.y << "," 
            << sprite.angle << "," 
            << (sprite.isPenDown ? 1 : 0) << "," 
            << sprite.currentCostumeIndex << std::endl;
    
    for (const auto& costume : sprite.costumes) {
        outFile << costume << std::endl;
    }
    
    outFile.close();
}

// Load sprite data
Sprite load_sprite(std::string filename) {
    Sprite sprite;
    std::ifstream inFile("saves/" + filename + "_sprite.txt");
    
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not load sprite data" << std::endl;
        return sprite;
    }
    
    std::string line;
    
    // Header
    std::getline(inFile, line);
    
    std::getline(inFile, line);
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    
    if ((int)tokens.size() < MIN_SPRITE_DATA_COUNT) return sprite;

    sprite.x = std::stof(tokens[0]);
    sprite.y = std::stof(tokens[1]);
    sprite.angle = std::stof(tokens[2]);
    sprite.isPenDown = (std::stoi(tokens[3]) == 1);
    sprite.currentCostumeIndex = std::stoi(tokens[4]);
    
    while (std::getline(inFile, line)) {
        if (!line.empty()) {
            sprite.costumes.push_back(line);
        }
    }
    
    inFile.close();
    return sprite;
}
