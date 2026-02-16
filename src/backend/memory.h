#pragma once
#include "../common/definitions.h"

Block* create_block(BlockType t);
void delete_block(Block* b);
void delete_chain(Block* b);
int count_blocks(Block* b);
void reset_block_counter(int newValue = 1);
int get_block_counter();
void safe_delete_chain(Block* b, Block* parent_block);
