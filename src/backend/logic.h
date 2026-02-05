#pragma once
#include "../common/definitions.h"
#include "file_io.h"
#include "logger.h"

/**
 * @param top The block that will be on top
 * @param bottom The block that will be attached below top
 */
void connect_blocks(Block* top, Block* bottom);

/**
 * @param block The block to disconnect from its next
 * @return pointer to the disconnected block (previously block->next)
 */
Block* disconnect_next(Block* block);

/**
 * @param top The block above
 * @param middle The block to insert
 * @param bottom The block below (can be nullptr)
 */
void insert_block_between(Block* top, Block* middle, Block* bottom);

/**
 * @param top The top block in the proposed connection
 * @param bottom the bottom block in the proposed connection
 * @return true if connection would create a cycle, false otherwise
 */
bool would_create_cycle(Block* top, Block* bottom);

/**
 * @param block The starting block
 * @return Pointer to the last block in the chain
 */
Block* get_last_block(Block* block);

/**
 * @param block The starting block
 * @return The number of blocks in the chain
 */
int count_chain_length(Block* block);

/**
 * @param head The head of the chain to search
 * @param target The block to find
 * @return true if target is in the chain, false otherwise
 */
bool is_in_chain(Block* head, Block* target);
