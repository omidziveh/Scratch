#ifndef BLOCK_EXECUTOR_SENSING_H
#define BLOCK_EXECUTOR_SENSING_H

#include "../common/definitions.h"
#include <string>

struct ExecutionContext {
    Sprite* sprite;
    Stage* stage;
    int mouseX;
    int mouseY;
    float lastResult;
    bool lastCondition;
};

bool execute_sensing_block(Block* block, ExecutionContext& ctx);
bool execute_operator_block(Block* block, ExecutionContext& ctx);

#endif
