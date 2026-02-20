#pragma once
#include "../common/definitions.h"
#include <string>
#include <list>

bool save_project(const std::string& filename, const std::list<Block>& blocks, const Sprite& sprite);
bool load_project(const std::string& filename, std::list<Block>& blocks, Sprite& sprite, int& next_block_id);

std::string blocktype_to_string(BlockType type);
BlockType string_to_blocktype(const std::string& str);