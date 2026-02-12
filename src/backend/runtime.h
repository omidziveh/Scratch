#pragma once
#include "../common/definitions.h"
#include <vector>

enum RuntimeState {
    RUNTIME_STOPPED,
    RUNTIME_RUNNING,
    RUNTIME_PAUSED,
    RUNTIME_FINISHED
};

struct LoopContext {
    Block* loopBlock;
    int remainingIterations;
    int ticksWithoutWait;
};

struct Runtime {
    Block* currentBlock;
    Block* programHead;
    Sprite* targetSprite;
    RuntimeState state;
    std::vector<LoopContext> loopStack;
    int waitTicksRemaining;
    float tickRate;
    int totalTicksExecuted;
    int maxTicksAllowed;
    bool stepMode;
    bool breakpointHit;
    int ticksSinceLastWait;
    int watchdogThreshold;
    bool watchdogTriggered;
};

void runtime_init(Runtime* rt, Block* head, Sprite* sprite);
void runtime_reset(Runtime* rt);
void runtime_tick(Runtime* rt, Stage* stage);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);
void runtime_step(Runtime* rt, Stage* stage);
void runtime_set_step_mode(Runtime* rt, bool enabled);
const char* runtime_get_status(Runtime* rt);
bool runtime_check_watchdog(Runtime* rt);
void runtime_set_max_ticks(Runtime* rt, int maxTicks);
void runtime_set_watchdog_threshold(Runtime* rt, int threshold);
void runtime_reset_watchdog(Runtime* rt);

void execute_block(Runtime* rt, Block* b, Stage* stage);
void advance_to_next_block(Runtime* rt);

bool evaluate_condition(Runtime* rt, Block* b);
