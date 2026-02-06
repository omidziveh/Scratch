#pragma once
#include "../common/definitions.h"

void connect_blocks(Block* top, Block* bottom);
Block* disconnect_next(Block* block);
void insert_block_between(Block* top, Block* middle, Block* bottom);
bool would_create_cycle(Block* top, Block* bottom);
Block* get_last_block(Block* block);
int count_chain_length(Block* block);
