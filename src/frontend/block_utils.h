#ifndef BLOCK_UTILS_H
#define BLOCK_UTILS_H

#include "../common/definitions.h"
#include <string>

std::string blocktype_to_string(BlockType type);
BlockType string_to_blocktype(const std::string& str);
std::string block_get_label(BlockType type);
SDL_Color block_get_color(BlockType type);

#endif
