#include "custom_blocks.h"
#include "../utils/logger.h"
#include <algorithm>

static std::map<std::string, Block*> g_custom_block_definitions;

void custom_blocks_register(const std::string& name, Block* definition) {
    if (!definition) return;
    
    std::string cleanName = name;
    cleanName.erase(std::remove_if(cleanName.begin(), cleanName.end(), ::isspace), cleanName.end());

    g_custom_block_definitions[cleanName] = definition;
    log_info("Registered custom block definition: " + cleanName);
}

Block* custom_blocks_get(const std::string& name) {
    std::string cleanName = name;
    cleanName.erase(std::remove_if(cleanName.begin(), cleanName.end(), ::isspace), cleanName.end());

    auto it = g_custom_block_definitions.find(cleanName);
    if (it != g_custom_block_definitions.end()) {
        return it->second;
    }
    log_warning("Custom block definition not found: " + cleanName);
    return nullptr;
}

void custom_blocks_clear() {
    g_custom_block_definitions.clear();
    log_info("Cleared all custom block definitions");
}