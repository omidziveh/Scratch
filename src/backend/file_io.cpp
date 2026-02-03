#include "file_io.h"
#include "memory.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <iomanip>

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

// Helper function to recursively save all blocks
void save_block_recursive(std::ofstream& outFile, Block* current, int& blockCount) {
    if (current == nullptr) {
        return;
    }
    
        blockCount++;
        log_block_info(current, "  Saving: ");
        
        outFile << current->id << ","
                << blocktype_to_string(current->type) << ","
            << std::fixed << std::setprecision(2) << current->x << ","
            << std::fixed << std::setprecision(2) << current->y << ","
            << std::fixed << std::setprecision(2) << current->width << ","
            << std::fixed << std::setprecision(2) << current->height << ",";
        
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
        
        // Save inner chain
        if (current->inner != nullptr) {
        save_block_recursive(outFile, current->inner, blockCount);
    }
    
    // Recursively save next blocks
    if (current->next != nullptr) {
        save_block_recursive(outFile, current->next, blockCount);
    }
}

// Save block chain to file
bool save_to_file(Block* head, std::string filename) {
    if (head == nullptr) {
        log_warning("Attempting to save empty project (NULL head)");
        return false;
    }
    
    log_info("Starting save process for " + filename);
    
    std::ofstream outFile("saves/" + filename);
    
    if (!outFile.is_open()) {
        log_error("Could not open file for writing: saves/" + filename);
        return false;
    }
    
    log_debug("File opened successfully");

    // Write header
    outFile << "# Scratch Project Save File" << std::endl;
    outFile << "# Format: ID,TYPE,X,Y,WIDTH,HEIGHT,NEXT_ID,INNER_ID,ARG_COUNT,ARG1,ARG2,..." << std::endl;
    outFile << "# Position data (X,Y) is preserved for drag-and-drop sync" << std::endl;
    outFile << "# --------------------------------------------------" << std::endl;
    
    // Save all blocks recursively
    int blockCount = 0;
    Block* current = head;
    
    // First pass: save all top-level blocks
    while (current != nullptr) {
        save_block_recursive(outFile, current, blockCount);
        
        // Move to next top-level block (skip the chain we just saved)
        Block* temp = current;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        
        // Check if there's another independent chain
        // This assumes your frontend might have multiple independent chains
        break; // For single chain, break here
    }
    
    outFile.close();
    log_success("Project saved successfully: saves/" + filename + " (" + std::to_string(blockCount) + " blocks)");
    return true;
}

// Load a block chain from a file
Block* load_from_file(std::string filename) {
    log_info("Starting load process for: " + filename);
    
    std::ifstream inFile("saves/" + filename);
    
    if (!inFile.is_open()) {
        log_error("Could not open file for reading: saves/" + filename);
        return nullptr;
    }
    
    std::map<int, Block*> blockMap;
    std::map<int, int> nextIdMap;
    std::map<int, int> innerIdMap;
    
    Block* head = nullptr;
    std::string line;
    int lineNumber = 0;
    int blocksLoaded = 0;
    int maxId = 0;  // Track maximum ID for counter reset
    
    while (std::getline(inFile, line)) {
        lineNumber++;
        
        // Skip header and empty lines
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
            log_warning("Line " + std::to_string(lineNumber) + " has insufficient data, skipping");
            continue;
        }
        
        try {
            // Parse block data
            int id = std::stoi(tokens[0]);
            BlockType type = string_to_blocktype(tokens[1]);
            float x = std::stof(tokens[2]);
            float y = std::stof(tokens[3]);
            float width = std::stof(tokens[4]);
            float height = std::stof(tokens[5]);
            int nextId = std::stoi(tokens[6]);
            int innerId = std::stoi(tokens[7]);
            int argCount = std::stoi(tokens[8]);
            
            // Track max ID
            if (id > maxId) {
                maxId = id;
            }
            
            Block* block = create_block(type);
            block->id = id;
            block->x = x;
            block->y = y;
            block->width = width;
            block->height = height;
            
                // Restore arguments
            block->args.clear();
            for (int i = 0; i < argCount && (MIN_BLOCK_DATA_COUNT + i) < (int)tokens.size(); i++) {
                block->args.push_back(tokens[MIN_BLOCK_DATA_COUNT + i]);
            }
            
            blockMap[id] = block;
            nextIdMap[id] = nextId;
            innerIdMap[id] = innerId;
            
                blocksLoaded++;
                
                log_debug("Loaded Block #" + std::to_string(id) + 
                        " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            
        } catch (const std::exception& e) {
            log_error("Error parsing line " + std::to_string(lineNumber) + ": " + e.what());
            continue;
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
    
    // Third pass: find the true head (block not referenced by any other block)
    std::set<int> referencedIds;
    for (auto& pair : blockMap) {
        if (nextIdMap[pair.first] != -1) {
            referencedIds.insert(nextIdMap[pair.first]);
        }
        if (innerIdMap[pair.first] != -1) {
            referencedIds.insert(innerIdMap[pair.first]);
        }
    }
    
    // Find block that is not referenced
    head = nullptr;
    for (auto& pair : blockMap) {
        if (referencedIds.find(pair.first) == referencedIds.end()) {
            head = pair.second;
            log_debug("Found head block: #" + std::to_string(head->id));
            break;
        }
    }
    
    if (head == nullptr && !blockMap.empty()) {
        // Fallback: if all blocks are referenced (circular?), use first block
        head = blockMap.begin()->second;
        log_warning("Could not find unreferenced head block, using first block");
    }
    
    // CRITICAL: Reset block ID counter to avoid ID conflicts with new blocks
    reset_block_counter(maxId + 1);
    log_debug("Reset block counter to " + std::to_string(maxId + 1));
    
    log_success("Project loaded successfully from: saves/" + filename + 
                " (" + std::to_string(blocksLoaded) + " blocks)");
    
    return head;
}

// Save sprite data
void save_sprite(const Sprite& sprite, std::string filename) {
    std::ofstream outFile("saves/" + filename + "_sprite.txt");
    
    if (!outFile.is_open()) {
        log_error("Could not save sprite data to: saves/" + filename + "_sprite.txt");
        return;
    }
    
    log_info("Saving sprite data for: " + filename);
    
    outFile << "# Sprite Data" << std::endl;
    outFile << std::fixed << std::setprecision(2);
    outFile << sprite.x << "," 
            << sprite.y << "," 
            << sprite.angle << "," 
            << (sprite.isPenDown ? 1 : 0) << "," 
            << sprite.currentCostumeIndex << std::endl;
    
    for (const auto& costume : sprite.costumes) {
        outFile << costume << std::endl;
    }
    
    outFile.close();
    log_success("Sprite data saved successfully");
}

// Load sprite data
Sprite load_sprite(std::string filename) {
    Sprite sprite;
    std::ifstream inFile("saves/" + filename + "_sprite.txt");
    
    if (!inFile.is_open()) {
        log_error("Could not load sprite data from: saves/" + filename + "_sprite.txt");
        return sprite;
    }
    
    log_info("Loading sprite data for: " + filename);
    
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
    
    if ((int)tokens.size() < MIN_SPRITE_DATA_COUNT) {
        log_error("Invalid sprite data format");
        return sprite;
    }

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
    log_success("Sprite data loaded successfully");
    return sprite;
}
