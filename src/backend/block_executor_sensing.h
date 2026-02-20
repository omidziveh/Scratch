#ifndef BLOCK_EXECUTOR_SENSING_H
#define BLOCK_EXECUTOR_SENSING_H

#include "../common/definitions.h"
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>


bool execute_sensing_block(Block* block, ExecutionContext& ctx);
bool execute_operator_block(Block* block, ExecutionContext& ctx);

#endif
