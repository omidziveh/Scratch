#ifndef BLOCK_EXECUTOR_LOOKS_H
#define BLOCK_EXECUTOR_LOOKS_H

#include "../common/definitions.h"

struct ExecutionContext {
    Sprite* sprite;
    Stage*  stage;
    int     mouseX;
    int     mouseY;
    float   lastResult;
    bool    lastCondition;

    ExecutionContext()
        : sprite(nullptr)
        , stage(nullptr)
        , mouseX(0)
        , mouseY(0)
        , lastResult(0.0f)
        , lastCondition(false)
    {}
};

bool execute_looks_block(Block* block, ExecutionContext& ctx);

#endif
