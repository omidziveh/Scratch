#pragma once
#include "../common/definitions.h"

void connect_blocks(Block* top, Block* bottom);
void connect_inner(Block* container, Block* child);
void insert_block_between(Block* top, Block* middle, Block* bottom);
bool would_create_cycle(Block* top, Block* bottom);
Block* disconnect_next(Block* block);
Block* disconnect_inner(Block* block);
Block* disconnect_from_parent(Block* block);
Block* get_last_block(Block* block);
Block* get_first_block(Block* block);
int count_chain_length(Block* block);
