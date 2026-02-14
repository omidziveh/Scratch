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
    rt->waitingForStep = false;
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
    rt->waitingForStep = false;
    rt->watchdogTriggered = false;
}

void runtime_start(Runtime* rt) {
    if (rt->state == RUNTIME_STOPPED || rt->state == RUNTIME_FINISHED) {
        rt->currentBlock = rt->programHead;
        rt->totalTicksExecuted = 0;
        rt->loopStack.clear();
        rt->ticksSinceLastWait = 0;
        rt->watchdogTriggered = false;
        rt->waitingForStep = false;
    }
    rt->state = RUNTIME_RUNNING;
    if (rt->stepMode) {
        rt->waitingForStep = true;
    }
    log_info("Runtime started");
}

void runtime_stop(Runtime* rt) {
    rt->state = RUNTIME_STOPPED;
    rt->waitingForStep = false;
    rt->watchdogTriggered = false;
    log_info("Runtime stopped");
}

void runtime_pause(Runtime* rt) {
    if (rt->state == RUNTIME_RUNNING) {
        rt->state = RUNTIME_PAUSED;
        rt->waitingForStep = false;
        log_info("Runtime paused");
    }
}

void runtime_resume(Runtime* rt) {
    if (rt->state == RUNTIME_PAUSED) {
        rt->state = RUNTIME_RUNNING;
        if (rt->stepMode) {
            rt->waitingForStep = true;
        }
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

void runtime_advance_step(Runtime* rt, Stage* stage) {
    if (!rt->stepMode || !rt->waitingForStep) return;
    if (rt->state != RUNTIME_RUNNING) return;
    if (!rt->currentBlock) {
        rt->state = RUNTIME_FINISHED;
        rt->waitingForStep = false;
        log_info("Program finished");
        return;
    }

    if (rt->waitTicksRemaining > 0) {
        rt->waitTicksRemaining--;
        rt->ticksSinceLastWait = 0;
        return;
    }

    if (runtime_check_watchdog(rt)) {
        rt->state = RUNTIME_STOPPED;
        rt->watchdogTriggered = true;
        rt->waitingForStep = false;
        log_error("Watchdog triggered - possible infinite loop detected");
        return;
    }

    if (rt->currentBlock->hasBreakpoint) {
        rt->breakpointHit = true;
        rt->state = RUNTIME_PAUSED;
        rt->waitingForStep = false;
        log_info("Breakpoint hit");
        return;
    }

    execute_block(rt, rt->currentBlock, stage);
    advance_to_next_block(rt);
    rt->totalTicksExecuted++;
    rt->ticksSinceLastWait++;

    if (!rt->currentBlock) {
        rt->state = RUNTIME_FINISHED;
        rt->waitingForStep = false;
        log_info("Program finished");
    }
}

void runtime_set_step_mode(Runtime* rt, bool enabled) {
    rt->stepMode = enabled;
    if (enabled) {
        rt->waitingForStep = true;
        if (rt->state == RUNTIME_RUNNING) {
            rt->state = RUNTIME_RUNNING;
        }
    } else {
        rt->waitingForStep = false;
    }
    log_info(enabled ? "Step mode ON" : "Step mode OFF");
}

bool runtime_is_waiting_for_step(Runtime* rt) {
    return rt->stepMode && rt->waitingForStep && rt->state == RUNTIME_RUNNING;
}

void runtime_tick(Runtime* rt, Stage* stage) {
    if (rt->state != RUNTIME_RUNNING) return;

    if (rt->stepMode && rt->waitingForStep) {
        return;
    }

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

    bool isLastBlock = (rt->currentBlock->next == nullptr && rt->loopStack.empty());
    execute_block(rt, rt->currentBlock, stage);
    if (isLastBlock) {
        rt->waitTicksRemaining = 30;
    }
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
    if (rt->stepMode && rt->waitingForStep) return "STEP_WAITING";

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

void pen_draw_line(SDL_Renderer* renderer, Sprite* sprite, float oldX, float oldY, float newX, float newY) {
    if (!renderer || !sprite) return;
    SDL_SetRenderDrawColor(renderer, sprite->penR, sprite->penG, sprite->penB, 255);
    int size = sprite->penSize;
    if (size < 1) size = 1;
    for (int w = -(size / 2); w <= (size / 2); w++) {
        SDL_RenderDrawLine(renderer,
            (int)oldX + w, (int)oldY,
            (int)newX + w, (int)newY);
        SDL_RenderDrawLine(renderer,
            (int)oldX, (int)oldY + w,
            (int)newX, (int)newY + w);
    }
}

void pen_clear(SDL_Renderer* renderer, Stage* stage) {
    if (!renderer || !stage) return;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect rect = {STAGE_X, STAGE_Y, STAGE_WIDTH, STAGE_HEIGHT};
    SDL_RenderFillRect(renderer, &rect);
}

void pen_stamp(SDL_Renderer* renderer, Sprite* sprite) {
    if (!renderer || !sprite) return;
    SDL_SetRenderDrawColor(renderer, sprite->penR, sprite->penG, sprite->penB, 255);
    int stampSize = sprite->penSize * 5;
    if (stampSize < 10) stampSize = 10;
    SDL_Rect r;
    r.x = (int)sprite->x - stampSize / 2;
    r.y = (int)sprite->y - stampSize / 2;
    r.w = stampSize;
    r.h = stampSize;
    SDL_RenderFillRect(renderer, &r);
}

void execute_block(Runtime* rt, Block* b, Stage* stage) {
    if (!b || !rt->targetSprite) return;

    b->is_running = true;
    b->glow_start_time = SDL_GetTicks();

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

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            float rad = rt->targetSprite->angle * 3.14159265f / 180.0f;
            rt->targetSprite->x += steps * std::cos(rad);
            rt->targetSprite->y += steps * std::sin(rad);
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
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

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x = gotoX;
            rt->targetSprite->y = gotoY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
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

        case CMD_SET_X: {
            float newX = 0.0f;
            if (!b->args.empty()) newX = (float)std::atof(b->args[0].c_str());

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x = STAGE_X + STAGE_WIDTH / 2.0f + newX;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
            break;
        }

        case CMD_SET_Y: {
            float newY = 0.0f;
            if (!b->args.empty()) newY = (float)std::atof(b->args[0].c_str());

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->y = STAGE_Y + STAGE_HEIGHT / 2.0f - newY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
            break;
        }

        case CMD_CHANGE_X: {
            float deltaX = 0.0f;
            if (!b->args.empty()) deltaX = (float)std::atof(b->args[0].c_str());

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x += deltaX;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
            break;
        }

        case CMD_CHANGE_Y: {
            float deltaY = 0.0f;
            if (!b->args.empty()) deltaY = (float)std::atof(b->args[0].c_str());

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->y -= deltaY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, rt->targetSprite, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y);
            }
            break;
        }

        case CMD_EVENT_CLICK: {
            break;
        }

        case CMD_PEN_DOWN: {
            rt->targetSprite->isPenDown = 1;
            log_info("Pen down");
            break;
        }

        case CMD_PEN_UP: {
            rt->targetSprite->isPenDown = 0;
            log_info("Pen up");
            break;
        }

        case CMD_PEN_CLEAR: {
            if (stage && stage->renderer) {
                pen_clear(stage->renderer, stage);
            }
            log_info("Pen cleared");
            break;
        }

        case CMD_PEN_SET_COLOR: {
            if (!b->args.empty()) {
                int colorVal = std::atoi(b->args[0].c_str());
                int r = (colorVal >> 16) & 0xFF;
                int g = (colorVal >> 8) & 0xFF;
                int bVal = colorVal & 0xFF;
                if (r == 0 && g == 0 && bVal == 0 && colorVal != 0) {
                    rt->targetSprite->penR = (Uint8)(colorVal % 256);
                    rt->targetSprite->penG = (Uint8)(colorVal % 256);
                    rt->targetSprite->penB = (Uint8)(colorVal % 256);
                } else {
                    rt->targetSprite->penR = (Uint8)r;
                    rt->targetSprite->penG = (Uint8)g;
                    rt->targetSprite->penB = (Uint8)bVal;
                }
            }
            log_info("Pen color set");
            break;
        }

        case CMD_PEN_SET_SIZE: {
            if (!b->args.empty()) {
                int size = std::atoi(b->args[0].c_str());
                if (size < 1) size = 1;
                if (size > 100) size = 100;
                rt->targetSprite->penSize = size;
            }
            log_info("Pen size set");
            break;
        }

        case CMD_PEN_STAMP: {
            if (stage && stage->renderer) {
                pen_stamp(stage->renderer, rt->targetSprite);
            }
            log_info("Stamp");
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

    current->is_running = false;

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
