#include "runtime.h"
#include "logger.h"
#include "../common/globals.h"
#include <cmath>
#include <cstdlib>
#include <sstream>

void runtime_init(Runtime* rt, Block* head, Sprite* sprite) {
    rt->programHead = head;
    rt->currentBlock = head;
    rt->targetSprite = sprite;
    rt->state = RUNTIME_STOPPED;
    rt->loopStack.clear();
    rt->waitTicksRemaining = 0;
    rt->tickRate = (float)DEFAULT_TICK_RATE;
    rt->totalTicksExecuted = 0;
    rt->maxTicksAllowed = DEFAULT_MAX_TICKS;
    rt->stepMode = false;
    rt->breakpointHit = false;
}

void runtime_reset(Runtime* rt) {
    rt->currentBlock = rt->programHead;
    rt->state = RUNTIME_STOPPED;
    rt->loopStack.clear();
    rt->waitTicksRemaining = 0;
    rt->totalTicksExecuted = 0;
    rt->breakpointHit = false;
}

void runtime_start(Runtime* rt) {
    if (rt->state == RUNTIME_STOPPED || rt->state == RUNTIME_FINISHED) {
        rt->currentBlock = rt->programHead;
        rt->totalTicksExecuted = 0;
        rt->loopStack.clear();
    }
    rt->state = RUNTIME_RUNNING;
    log_info("Runtime started");
}

void runtime_stop(Runtime* rt) {
    rt->state = RUNTIME_STOPPED;
    log_info("Runtime stopped");
}

void runtime_pause(Runtime* rt) {
    if (rt->state == RUNTIME_RUNNING) {
        rt->state = RUNTIME_PAUSED;
        log_info("Runtime paused");
    }
}

void runtime_resume(Runtime* rt) {
    if (rt->state == RUNTIME_PAUSED) {
        rt->state = RUNTIME_RUNNING;
        log_info("Runtime resumed");
    }
}

void runtime_step(Runtime* rt) {
    if (rt->currentBlock && (rt->state == RUNTIME_PAUSED || rt->stepMode)) {
        execute_block(rt, rt->currentBlock);
        advance_to_next_block(rt);
        rt->totalTicksExecuted++;
    }
}

void runtime_set_step_mode(Runtime* rt, bool enabled) {
    rt->stepMode = enabled;
    if (enabled) {
        rt->state = RUNTIME_PAUSED;
    }
    log_info(enabled ? "Step mode ON" : "Step mode OFF");
}

bool runtime_check_watchdog(Runtime* rt) {
    if (rt->totalTicksExecuted >= rt->maxTicksAllowed) {
        log_error("Watchdog triggered - possible infinite loop");
        rt->state = RUNTIME_STOPPED;
        return true;
    }
    return false;
}

void runtime_set_max_ticks(Runtime* rt, int maxTicks) {
    rt->maxTicksAllowed = maxTicks;
}

const char* runtime_get_status(Runtime* rt) {
    static char buffer[256];
    const char* stateStr = "UNKNOWN";

    switch (rt->state) {
        case RUNTIME_STOPPED: stateStr = "STOPPED"; break;
        case RUNTIME_RUNNING: stateStr = "RUNNING"; break;
        case RUNTIME_PAUSED: stateStr = "PAUSED"; break;
        case RUNTIME_FINISHED: stateStr = "FINISHED"; break;
    }

    int blockId = rt->currentBlock ? rt->currentBlock->id : -1;
    snprintf(buffer, sizeof(buffer),
             "State: %s | Block: %d | Ticks: %d | LoopDepth: %d",
             stateStr, blockId, rt->totalTicksExecuted, (int)rt->loopStack.size());

    return buffer;
}

void runtime_tick(Runtime* rt) {
    if (rt->state != RUNTIME_RUNNING) return;
    if (!rt->currentBlock) {
        rt->state = RUNTIME_FINISHED;
        log_info("Program finished");
        return;
    }

    if (runtime_check_watchdog(rt)) return;

    if (rt->waitTicksRemaining > 0) {
        rt->waitTicksRemaining--;
        return;
    }

    execute_block(rt, rt->currentBlock);
    advance_to_next_block(rt);
    rt->totalTicksExecuted++;

    if (rt->stepMode) {
        rt->state = RUNTIME_PAUSED;
    }
}

void execute_block(Runtime* rt, Block* b) {
    if (!b || !rt->targetSprite) return;

    Sprite* s = rt->targetSprite;

    switch (b->type) {
        case CMD_MOVE: {
            float steps = 10.0f;
            if (!b->args.empty()) steps = (float)std::atof(b->args[0].c_str());

            float rad = s->angle * (3.14159265f / 180.0f);
            float newX = s->x + steps * std::cos(rad);
            float newY = s->y + steps * std::sin(rad);

            if (newX >= 0 && newX <= WINDOW_WIDTH) s->x = newX;
            if (newY >= 0 && newY <= WINDOW_HEIGHT) s->y = newY;
            break;
        }

        case CMD_TURN: {
            float degrees = 15.0f;
            if (!b->args.empty()) degrees = (float)std::atof(b->args[0].c_str());
            s->angle += degrees;
            while (s->angle >= 360.0f) s->angle -= 360.0f;
            while (s->angle < 0.0f) s->angle += 360.0f;
            break;
        }

        case CMD_GOTO: {
            if (b->args.size() >= 2) {
                s->x = (float)std::atof(b->args[0].c_str());
                s->y = (float)std::atof(b->args[1].c_str());
            }
            break;
        }

        case CMD_WAIT: {
            float seconds = 1.0f;
            if (!b->args.empty()) seconds = (float)std::atof(b->args[0].c_str());
            rt->waitTicksRemaining = (int)(seconds * rt->tickRate);
            break;
        }

        case CMD_SAY: {
            std::string msg = "Hello!";
            if (!b->args.empty()) msg = b->args[0];
            log_info("Sprite says: " + msg);
            break;
        }

        case CMD_REPEAT: {
            int times = 10;
            if (!b->args.empty()) times = std::atoi(b->args[0].c_str());

            LoopContext ctx;
            ctx.loopBlock = b;
            ctx.remainingIterations = times;
            rt->loopStack.push_back(ctx);
            break;
        }

        case CMD_IF: {
            bool condition = true;
            if (condition && b->inner) {
                LoopContext ctx;
                ctx.loopBlock = b;
                ctx.remainingIterations = 1;
                rt->loopStack.push_back(ctx);
            }
            break;
        }

        default:
            break;
    }
}

void advance_to_next_block(Runtime* rt) {
    if (!rt->currentBlock) return;

    Block* current = rt->currentBlock;

    if ((current->type == CMD_REPEAT || current->type == CMD_IF) && current->inner) {
        if (!rt->loopStack.empty()) {
            rt->currentBlock = current->inner;
            return;
        }
    }

    if (current->next) {
        rt->currentBlock = current->next;
        return;
    }

    while (!rt->loopStack.empty()) {
        LoopContext& ctx = rt->loopStack.back();
        ctx.remainingIterations--;

        if (ctx.remainingIterations > 0 && ctx.loopBlock->type == CMD_REPEAT) {
            rt->currentBlock = ctx.loopBlock->inner;
            return;
        }

        rt->loopStack.pop_back();

        if (ctx.loopBlock->next) {
            rt->currentBlock = ctx.loopBlock->next;
            return;
        }
    }

    rt->currentBlock = nullptr;
}
