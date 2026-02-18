#ifndef CUSTOM_BLOCKS_H
#define CUSTOM_BLOCKS_H

#include "../common/definitions.h"
#include <string>
#include <map>

void custom_blocks_register(const std::string& name, Block* definition);
Block* custom_blocks_get(const std::string& name);
void custom_blocks_clear();

#endif