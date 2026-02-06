#pragma once
#include "../common/definitions.h"
#include <string>

bool save_to_file(Block* head, std::string filename);
Block* load_from_file(std::string filename);
std::string blocktype_to_string(BlockType type);
BlockType string_to_blocktype(const std::string& str);
void save_sprite(const Sprite& sprite, std::string filename);
Sprite load_sprite(std::string filename);
