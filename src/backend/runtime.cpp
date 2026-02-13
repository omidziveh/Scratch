#include "runtime.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include <cmath>
#include <cstdlib>
#include <sstream>
#include "sensing.h"

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
    rt->ticksSinceLastWait = 0;
    rt->watchdogThreshold = DEFAULT_WATCHDOG_THRESHOLD;
    rt->watchdogTriggered = false;
}

void runtime_reset(Runtime* rt) {
    rt->currentBlock = rt->programHead;
    rt->state = RUNTIME_STOPPED;
    rt->loopStack.clear();
    rt->waitTicksRemaining = 0;
    rt->totalTicksExecuted = 0;
    rt->breakpointHit = false;
    rt->ticksSinceLastWait = 0;
    rt->watchdogTriggered = false;
}

void runtime_start(Runtime* rt) {
    if (rt->state == RUNTIME_STOPPED || rt->state == RUNTIME_FINISHED) {
        rt->currentBlock = rt->programHead;
        rt->totalTicksExecuted = 0;
        rt->loopStack.clear();
        rt->ticksSinceLastWait = 0;
        rt->watchdogTriggered = false;
    }
    rt->state = RUNTIME_RUNNING;
    log_info("Runtime started");
}

void runtime_stop(Runtime* rt) {
    rt->state = RUNTIME_STOPPED;
    rt->watchdogTriggered = false;
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

void runtime_step(Runtime* rt, Stage* stage) {
    if (rt->currentBlock && (rt->state == RUNTIME_PAUSED || rt->stepMode)) {
        execute_block(rt, rt->currentBlock, stage);
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

void runtime_tick(Runtime* rt, Stage* stage) {
    if (rt->state != RUNTIME_RUNNING) return;

    if (rt->waitTicksRemaining > 0) {
        rt->waitTicksRemaining--;
        rt->ticksSinceLastWait = 0;
        return;
    }

    if (!rt->currentBlock) {
        rt->state = RUNTIME_FINISHED;
        log_info("Program finished");
        return;
    }

    if (runtime_check_watchdog(rt)) {
        rt->state = RUNTIME_STOPPED;
        rt->watchdogTriggered = true;
        log_error("Watchdog triggered - possible infinite loop detected");
        return;
    }

    if (rt->currentBlock->hasBreakpoint) {
        rt->breakpointHit = true;
        rt->state = RUNTIME_PAUSED;
        log_info("Breakpoint hit");
        return;
    }

    execute_block(rt, rt->currentBlock, stage);
    advance_to_next_block(rt);
    rt->totalTicksExecuted++;
    rt->ticksSinceLastWait++;
}

bool runtime_check_watchdog(Runtime* rt) {
    if (rt->totalTicksExecuted >= rt->maxTicksAllowed) {
        log_error("Max ticks exceeded");
        return true;
    }

    if (rt->ticksSinceLastWait >= rt->watchdogThreshold) {
        log_error("Too many ticks without wait");
        return true;
    }

    for (size_t i = 0; i < rt->loopStack.size(); i++) {
        if (rt->loopStack[i].ticksWithoutWait >= LOOP_WATCHDOG_LIMIT) {
            log_error("Loop running too long without wait");
            return true;
        }
    }

    return false;
}

void runtime_set_max_ticks(Runtime* rt, int maxTicks) {
    rt->maxTicksAllowed = maxTicks;
}

void runtime_set_watchdog_threshold(Runtime* rt, int threshold) {
    rt->watchdogThreshold = threshold;
}

void runtime_reset_watchdog(Runtime* rt) {
    rt->ticksSinceLastWait = 0;
    for (size_t i = 0; i < rt->loopStack.size(); i++) {
        rt->loopStack[i].ticksWithoutWait = 0;
    }
}

const char* runtime_get_status(Runtime* rt) {
    if (rt->watchdogTriggered) return "WATCHDOG_TRIGGERED";
    
    switch (rt->state) {
        case RUNTIME_STOPPED: return "STOPPED";
        case RUNTIME_RUNNING: return "RUNNING";
        case RUNTIME_PAUSED: return "PAUSED";
        case RUNTIME_FINISHED: return "FINISHED";
        default: return "UNKNOWN";
    }
}

bool evaluate_condition(Runtime* rt, Block* b) {
    if (b->args.empty()) return true;
    
    std::string conditionStr = b->args[0];
    
    if (conditionStr == "true" || conditionStr == "1") return true;
    if (conditionStr == "false" || conditionStr == "0") return false;
    
    if (b->args.size() >= 3) {
        float left = 0.0f;
        float right = 0.0f;
        std::string op = b->args[1];
        
        if (b->args[0] == "x" && rt->targetSprite) {
            left = rt->targetSprite->x;
        } else if (b->args[0] == "y" && rt->targetSprite) {
            left = rt->targetSprite->y;
        } else {
            left = (float)std::atof(b->args[0].c_str());
        }
        
        if (b->args[2] == "x" && rt->targetSprite) {
            right = rt->targetSprite->x;
        } else if (b->args[2] == "y" && rt->targetSprite) {
            right = rt->targetSprite->y;
        } else {
            right = (float)std::atof(b->args[2].c_str());
        }
        
        if (op == ">" || op == "gt") return left > right;
        if (op == "<" || op == "lt") return left < right;
        if (op == ">=" || op == "gte") return left >= right;
        if (op == "<=" || op == "lte") return left <= right;
        if (op == "==" || op == "eq") return std::fabs(left - right) < 0.0001f;
        if (op == "!=" || op == "neq") return std::fabs(left - right) >= 0.0001f;
    }
    
    return true;
}

void execute_block(Runtime* rt, Block* b, Stage* stage) {
    if (!b || !rt->targetSprite) return;

    for (size_t i = 0; i < rt->loopStack.size(); i++) {
        rt->loopStack[i].ticksWithoutWait++;
    }

    std::stringstream ss;
    ss << "Executing block #" << b->id << " type=" << b->type;
    log_debug(ss.str());

    bool hasChanged = false;

    switch (b->type) {
        case CMD_MOVE: {
            float steps = 10.0f;
            if (!b->args.empty()) steps = (float)std::atof(b->args[0].c_str());
            float rad = rt->targetSprite->angle * 3.14159265f / 180.0f;
            rt->targetSprite->x += steps * std::cos(rad);
            rt->targetSprite->y += steps * std::sin(rad);
            hasChanged = true;
            break;
        }

        case CMD_TURN: {
            float degrees = 15.0f;
            if (!b->args.empty()) degrees = (float)std::atof(b->args[0].c_str());
            rt->targetSprite->angle += degrees;
            
            while (rt->targetSprite->angle >= 360.0f) rt->targetSprite->angle -= 360.0f;
            while (rt->targetSprite->angle < 0.0f) rt->targetSprite->angle += 360.0f;
            break;
        }

        case CMD_GOTO: {
            float gotoX = 0.0f;
            float gotoY = 0.0f;
            if (b->args.size() >= 2) {
                gotoX = (float)std::atof(b->args[0].c_str());
                gotoY = (float)std::atof(b->args[1].c_str());
            }
            rt->targetSprite->x = gotoX;
            rt->targetSprite->y = gotoY;
            hasChanged = true;
            break;
        }

        case CMD_WAIT: {
            float seconds = 1.0f;
            if (!b->args.empty()) seconds = (float)std::atof(b->args[0].c_str());
            rt->waitTicksRemaining = (int)(seconds * rt->tickRate);
            
            rt->ticksSinceLastWait = 0;
            for (size_t i = 0; i < rt->loopStack.size(); i++) {
                rt->loopStack[i].ticksWithoutWait = 0;
            }
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
            
            if (times <= 0) {
                log_warning("REPEAT with zero or negative count, skipping");
                break;
            }
            
            if (times > 100000) {
                log_warning("REPEAT count too high, capping at 100000");
                times = 100000;
            }

            LoopContext ctx;
            ctx.loopBlock = b;
            ctx.remainingIterations = times;
            ctx.ticksWithoutWait = 0;
            rt->loopStack.push_back(ctx);
            break;
        }

        case CMD_IF: {
            bool condition = evaluate_condition(rt, b);
            
            std::stringstream condLog;
            condLog << "IF condition evaluated to: " << (condition ? "true" : "false");
            log_debug(condLog.str());
            
            if (condition && b->inner) {
                LoopContext ctx;
                ctx.loopBlock = b;
                ctx.remainingIterations = 1;
                ctx.ticksWithoutWait = 0;
                rt->loopStack.push_back(ctx);
            }
            break;
        }

        case CMD_EVENT_CLICK: {
            break;
        }

        default:
            log_warning("Unknown block type encountered");
            break;
    }
    if (hasChanged) {
        clamp_sprite_to_stage(*rt->targetSprite, *stage);
    }
}

void advance_to_next_block(Runtime* rt) {
    if (!rt->currentBlock) return;

    Block* current = rt->currentBlock;

    if ((current->type == CMD_REPEAT || current->type == CMD_IF) && current->inner) {
        if (!rt->loopStack.empty() && rt->loopStack.back().loopBlock == current) {
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
            if (ctx.ticksWithoutWait >= LOOP_WATCHDOG_LIMIT) {
                log_error("Loop watchdog: forcing break");
                rt->loopStack.pop_back();
                rt->watchdogTriggered = true;
                rt->state = RUNTIME_STOPPED;
                return;
            }
            rt->currentBlock = ctx.loopBlock->inner;
            return;
        }

        Block* parentBlock = ctx.loopBlock;
        rt->loopStack.pop_back();

        if (parentBlock->next) {
            rt->currentBlock = parentBlock->next;
            return;
        }
    }

    rt->currentBlock = nullptr;
}
