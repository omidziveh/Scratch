#pragma once
#include "../common/definitions.h"
#include <string>

bool save_project(const std::string& filename, Block* head, const Sprite& sprite);

Block* load_project(const std::string& filename, Sprite& sprite);

std::string blocktype_to_string(BlockType type);
BlockType string_to_blocktype(const std::string& str);