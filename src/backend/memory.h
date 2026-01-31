#pragma once
#include "../common/definitions.h"

// Creates and initializes a new block with the given type
// Returns a pointer to the newly allocated block
Block* create_block(BlockType t);

// Recursively deletes a chain of blocks
void delete_chain(Block* b);

// Deletes a single block and its inner chain
void delete_block(Block* b);

// Utility function to count total blocks in a chain
int count_blocks(Block* b);
