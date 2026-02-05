#pragma once
#include "../common/definitions.h"
#include <string>

// Save a block chain to a file
bool save_to_file(Block* head, std::string filename);

// Load a block chain from a file
Block* load_from_file(std::string filename);

// Convert BlockType enum to string
std::string blocktype_to_string(BlockType type);

// Convert string to BlockType enum
BlockType string_to_blocktype(const std::string& str);

// Save sprite data
void save_sprite(const Sprite& sprite, std::string filename);

// Load sprite data
Sprite load_sprite(std::string filename);
