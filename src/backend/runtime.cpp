#include "runtime.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include "sensing.h"
#include "block_executor_sensing.h"
#include "block_executor_sound.h"
#include "block_executor_looks.h"
#include "custom_blocks.h"
#include "../frontend/pen.h"
#include <cstdlib>
#include <cmath>
#include <ctime>

float evaluate_block_argument(Runtime* rt, Block* host, int argIndex) {
    if (!host) return 0.0f;

    if (argIndex < (int)host->argBlocks.size() && host->argBlocks[argIndex] != nullptr) {
        Block* subBlock = host->argBlocks[argIndex];

        execute_block(rt, subBlock, rt->stage);
        float result = rt->lastResult;

        if (rt->lastError && rt->targetSprite) {
            rt->targetSprite->sayText = rt->lastErrorMessage;
            rt->targetSprite->sayStartTime = SDL_GetTicks();
            rt->lastError = false;
            rt->lastErrorMessage = "";
        }
        subBlock->is_running = false;
        return result;
    }

    if (argIndex < (int)host->args.size()) {
        return resolve_argument(rt, host->args[argIndex]);
    }

    return 0.0f;
}


std::string resolve_string_variable(Runtime* rt, const std::string& arg) {
if (!rt || !rt->targetSprite) return arg;

if (arg.find('%') == std::string::npos) {
return arg;
}

std::string result;
result.reserve(arg.size());
size_t i = 0;

while (i < arg.length()) {
if (arg[i] == '%') {
size_t start = i + 1;
size_t end = start;

while (end < arg.length() && (isalnum(arg[end]) || arg[end] == '_')) {
end++;
}

if (end > start) {
std::string varName = arg.substr(start, end - start);
bool found = false;

for (const auto& var : rt->targetSprite->variables) {
if (var.name == varName) {
result += var.value;
found = true;
break;
}
}

if (!found) {
result += arg.substr(i, end - i);
}

i = end;
} else {
result += '%';
i++;
}
} else {
result += arg[i];
i++;
}
}

return result;
}

float resolve_argument(Runtime* rt, const std::string& arg) {
    if (!rt || !rt->targetSprite) return 0.0f;

    try {
        size_t pos = 0;
        float val = std::stof(arg, &pos);
        if (pos == arg.size()) {
            return val;
        }
    } catch (...) {
}

std::string resolved = resolve_string_variable(rt, arg);

            try {
return std::stof(resolved);
            } catch (...) {
                return 0.0f;
        }
}

void sprite_set_variable(Sprite* sprite, const std::string& name, const std::string& value) {
    if (!sprite) return;
    for (auto& var : sprite->variables) {
        if (var.name == name) {
            var.value = value;
            log_info("Set var " + name + " = " + value);
            return;
        }
    }
    sprite->variables.push_back(Variable(name, value));
    log_info("Created var " + name + " = " + value);
}

void sprite_change_variable(Sprite* sprite, const std::string& name, float delta) {
    if (!sprite) return;
    for (auto& var : sprite->variables) {
        if (var.name == name) {
            try {
                float current = std::stof(var.value);
                var.value = std::to_string(current + delta);
                log_info("Variable '" + name + "' changed to " + var.value);
            } catch (...) {
                var.value = std::to_string(delta);
            }
            return;
        }
    }
    sprite->variables.push_back(Variable(name, std::to_string(delta)));
    log_info("Variable '" + name + "' created via change with value " + std::to_string(delta));
}

void runtime_init(Runtime* rt, Block* head, Sprite* sprite) {
    rt->programHead = head;
    rt->currentBlock = head;
    rt->targetSprite = sprite;
    rt->state = RUNTIME_STOPPED;
    rt->loopStack.clear();
    rt->waitTicksRemaining = 0;
    rt->tickRate = (float)DEFAULT_TICK_RATE;
    rt->totalTicksExecuted = 0;
    rt->maxTicksAllowed = 1000000;
    rt->stepMode = false;
    rt->breakpointHit = false;
    rt->waitingForStep = false;
    rt->ticksSinceLastWait = 0;
    rt->watchdogThreshold = DEFAULT_WATCHDOG_THRESHOLD;
    rt->watchdogTriggered = false;
    rt->mouseX = 0;
    rt->mouseY = 0;
    rt->lastResult = 0.0f;
    rt->stage = nullptr;
    rt->callStack.clear();
    rt->scopeStack.clear();
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
    rt->lastExecutedBlock = nullptr;
    rt->highlightDelayTicks = 0;

    rt->callStack.clear();
    rt->scopeStack.clear();
}
static void push_scope(Runtime* rt, const std::vector<std::string>& paramNames, const std::vector<std::string>& values) {
    std::map<std::string, std::string> snapshot;
    
    for (const auto& name : paramNames) {
        for (const auto& v : rt->targetSprite->variables) {
            if (v.name == name) {
                snapshot[name] = v.value;
                break;
            }
        }
        if (snapshot.find(name) == snapshot.end()) {
            snapshot[name] = ""; 
        }
    }
    
    rt->scopeStack.push_back(snapshot);
    
    for (size_t i = 0; i < paramNames.size(); i++) {
        std::string val = (i < values.size()) ? values[i] : "0";
        bool found = false;
        for (auto& v : rt->targetSprite->variables) {
            if (v.name == paramNames[i]) {
                v.value = val;
                found = true;
                break;
            }
        }
        if (!found) {
            rt->targetSprite->variables.push_back(Variable(paramNames[i], val));
        }
    }
    
    log_debug("Pushed scope, scopeStack size now: " + std::to_string(rt->scopeStack.size()));
}

static void pop_scope(Runtime* rt) {
    if (rt->scopeStack.empty()) {
        log_warning("pop_scope called with empty scopeStack!");
        return;
    }
    
    std::map<std::string, std::string> snapshot = rt->scopeStack.back();
    rt->scopeStack.pop_back();
    
    auto& vars = rt->targetSprite->variables;
    for (const auto& pair : snapshot) {
        if (pair.second.empty()) {
            vars.erase(std::remove_if(vars.begin(), vars.end(), 
                [&pair](const Variable& v) { return v.name == pair.first; }), vars.end());
            log_debug("Removed variable on scope pop: " + pair.first);
        } else {
            for (auto& v : vars) {
                if (v.name == pair.first) {
                    v.value = pair.second;
                    log_debug("Restored variable on scope pop: " + pair.first + " = " + pair.second);
                    break;
                }
            }
        }
    }
    
    log_debug("Popped scope, scopeStack size now: " + std::to_string(rt->scopeStack.size()));
}

void runtime_start(Runtime* rt) {
    if (rt->state == RUNTIME_STOPPED || rt->state == RUNTIME_FINISHED) {
        rt->currentBlock = rt->programHead;
        rt->totalTicksExecuted = 0;
        rt->loopStack.clear();
        rt->ticksSinceLastWait = 0;
        rt->watchdogTriggered = false;
        rt->waitingForStep = false;
        rt->scopeStack.clear();
        rt->waitTicksRemaining = 0;
    }
    rt->state = RUNTIME_RUNNING;
    if (rt->stepMode) {
        rt->waitingForStep = true;
    }
    log_info("Runtime started");
}

void runtime_stop(Runtime* rt) {
    if (rt->lastExecutedBlock) {
        rt->lastExecutedBlock->is_running = false;
        rt->lastExecutedBlock = nullptr;
    }
    rt->highlightDelayTicks = 0;
    rt->state = RUNTIME_STOPPED;
    rt->waitingForStep = false;
    rt->watchdogTriggered = false;
    rt->scopeStack.clear();
    rt->waitTicksRemaining = 0;
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

void runtime_advance_step(Runtime* rt, Stage* stage, int mouseX, int mouseY) {
    if (!rt->stepMode || !rt->waitingForStep) return;

    rt->mouseX = mouseX;
    rt->mouseY = mouseY;

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

        if (rt->waitTicksRemaining == 0) {
            advance_to_next_block(rt);
        }
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
    
    if (rt->waitTicksRemaining > 0) {
        return;
    }

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
    } else {
        rt->waitingForStep = false;
    }
    log_info(enabled ? "Step mode ON" : "Step mode OFF");
}

bool runtime_is_waiting_for_step(Runtime* rt) {
    return rt->stepMode && rt->waitingForStep && rt->state == RUNTIME_RUNNING;
}

void runtime_tick(Runtime* rt, Stage* stage, int mouseX, int mouseY) {
    if (rt->state != RUNTIME_RUNNING) return;

    rt->stage = stage;
    rt->mouseX = mouseX;
    rt->mouseY = mouseY;

    if (rt->stepMode && rt->waitingForStep) {
        return;
    }

    if (rt->highlightDelayTicks > 0) {
        rt->highlightDelayTicks--;
        
        if (rt->highlightDelayTicks == 0) {
            if (rt->lastExecutedBlock) {
                rt->lastExecutedBlock->is_running = false;
                rt->lastExecutedBlock = nullptr;
            }
            advance_to_next_block(rt);
            rt->totalTicksExecuted++;
            rt->ticksSinceLastWait++;
        }
        return; 
    }

    
    if (rt->waitTicksRemaining > 0) {
        rt->waitTicksRemaining--;
        rt->ticksSinceLastWait = 0;
        
        if (rt->waitTicksRemaining == 0) {
            if (rt->lastExecutedBlock) {
                rt->lastExecutedBlock->is_running = false;
                rt->lastExecutedBlock = nullptr;
            }
            advance_to_next_block(rt);
        }
        return;
    }


    if (!rt->currentBlock) {
        // خاموش کردن آخرین بلوک
        if (rt->lastExecutedBlock) {
            rt->lastExecutedBlock->is_running = false;
            rt->lastExecutedBlock = nullptr;
        }
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

    
    if (rt->lastExecutedBlock) {
        rt->lastExecutedBlock->is_running = false;
        rt->lastExecutedBlock = nullptr;
    }

    Block* current = rt->currentBlock;
    
    execute_block(rt, current, stage);

    rt->lastExecutedBlock = current;

    if (rt->waitTicksRemaining > 0) {
        return;
    }

    bool isLastBlock = (current->next == nullptr && rt->loopStack.empty());
    if (isLastBlock) {
        rt->highlightDelayTicks = rt->highlightDelayDuration;
        return;
    }
    rt->highlightDelayTicks = rt->highlightDelayDuration;
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
    if (!b->argBlocks.empty() && b->argBlocks[0]) {
        float val = evaluate_block_argument(rt, b, 0);
        return (val != 0.0f);
    }
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
left = resolve_argument(rt, b->args[0]);
        }

        if (b->args[2] == "x" && rt->targetSprite) {
            right = rt->targetSprite->x;
        } else if (b->args[2] == "y" && rt->targetSprite) {
            right = rt->targetSprite->y;
        } else {
right = resolve_argument(rt, b->args[2]);
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

    b->has_executed = true;
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
        // Motion:
        case CMD_MOVE: {
            float steps = evaluate_block_argument(rt, b, 0);
            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            float rad = rt->targetSprite->angle * 3.14159265f / 180.0f;
            rt->targetSprite->x += steps * std::cos(rad);
            rt->targetSprite->y += steps * std::sin(rad);
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }
        case CMD_TURN: {
            float degrees = evaluate_block_argument(rt, b, 0);
            rt->targetSprite->angle += degrees;
            
            while (rt->targetSprite->angle >= 360.0f) rt->targetSprite->angle -= 360.0f;
            while (rt->targetSprite->angle < 0.0f) rt->targetSprite->angle += 360.0f;
            break;
        }
        case CMD_GOTO: {
            float gotoX = evaluate_block_argument(rt, b, 0);
            float gotoY = evaluate_block_argument(rt, b, 1);

            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x = gotoX;
            rt->targetSprite->y = gotoY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }

        // Control:
        case CMD_WAIT: {
            float seconds = evaluate_block_argument(rt, b, 0);
            int ticks = (int)(seconds * rt->tickRate);
            if (ticks < 1 && seconds > 0) ticks = 1;
            
            rt->waitTicksRemaining = ticks;

            rt->ticksSinceLastWait = 0;
            for (size_t i = 0; i < rt->loopStack.size(); i++) {
                rt->loopStack[i].ticksWithoutWait = 0;
            }
            break;
        }
        case CMD_SAY: {
            std::string msg = "Hello!";

            if (!b->argBlocks.empty() && b->argBlocks[0]) {
                execute_block(rt, b->argBlocks[0], stage);
                
                if (rt->targetSprite && rt->targetSprite->sayText.find("Error!") == 0) {
                    log_info("Sprite shows error: " + rt->targetSprite->sayText);
                    b->argBlocks[0]->is_running = false;
                    break;
                }                
                if (!rt->lastStringResult.empty()) {
                    msg = rt->lastStringResult;
                } else {
                    msg = std::to_string(rt->lastResult);
                }

                b->argBlocks[0]->is_running = false;
            }
            else if (!b->args.empty()) {
                msg = resolve_string_variable(rt, b->args[0]);
            }

            rt->targetSprite->sayText = msg;
            rt->targetSprite->sayStartTime = SDL_GetTicks();
            log_info("Sprite says: " + msg);
            break;
        }

        case CMD_REPEAT: {
            int times = (int)evaluate_block_argument(rt, b, 0);
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

        // Position:
        case CMD_SET_X: {
            float newX = evaluate_block_argument(rt, b, 0);
            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x = STAGE_X + STAGE_WIDTH / 2.0f + newX;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }
        case CMD_SET_Y: {
            float newY = evaluate_block_argument(rt, b, 0);
            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->y = STAGE_Y + STAGE_HEIGHT / 2.0f - newY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }
        case CMD_CHANGE_X: {
            float deltaX = evaluate_block_argument(rt, b, 0);
            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->x += deltaX;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }
        case CMD_CHANGE_Y: {
            float deltaY = evaluate_block_argument(rt, b, 0);
            float oldX = rt->targetSprite->x;
            float oldY = rt->targetSprite->y;

            rt->targetSprite->y -= deltaY;
            hasChanged = true;

            if (rt->targetSprite->isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY,
                              rt->targetSprite->x, rt->targetSprite->y, *rt->targetSprite);
            }
            break;
        }

        // Variables:
        case CMD_SET_VAR: {
            if (b->args.size() >= 2) {
                std::string name = b->args[0];
                float value = evaluate_block_argument(rt, b, 1);
                
                sprite_set_variable(rt->targetSprite, name, std::to_string(value));
            } else {
                log_warning("Set variable block missing arguments");
            }
            break;
        }
        case CMD_CHANGE_VAR: {
            if (b->args.size() >= 1) {
                std::string name = b->args[0];
                float delta = evaluate_block_argument(rt, b, 1);
                sprite_change_variable(rt->targetSprite, name, delta);
            }
            break;
        }


        // Pen:
        case CMD_PEN_DOWN: {
            rt->targetSprite->isPenDown = 1;
            rt->targetSprite->prevPenX = rt->targetSprite->x;
            rt->targetSprite->prevPenY = rt->targetSprite->y;
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
                pen_clear(stage->renderer);
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
                pen_stamp(stage->renderer, *rt->targetSprite);
            }
            log_info("Stamp");
            break;
        }

        // Looks:
        case CMD_SWITCH_COSTUME:
        case CMD_NEXT_COSTUME:
        case CMD_SET_SIZE:
        case CMD_CHANGE_SIZE:
        case CMD_SHOW:
        case CMD_HIDE: {
            ExecutionContext ctx;
            ctx.sprite = rt->targetSprite;
            ctx.stage = stage;
            ctx.mouseX = rt->mouseX;
            ctx.mouseY = rt->mouseY;
            execute_looks_block(b, ctx);
            break;
        }

        // Sounds:
        case CMD_PLAY_SOUND: {
            execute_play_sound(b, *rt->targetSprite);
            break;
        }
        case CMD_STOP_ALL_SOUNDS: {
            execute_stop_all_sounds(b, *rt->targetSprite);
            break;
        }
        case CMD_CHANGE_VOLUME: {
            execute_change_volume(b, *rt->targetSprite);
            break;
        }
        case CMD_SET_VOLUME: {
            execute_set_volume(b, *rt->targetSprite);
            break;
        }

        // Sensing:
        case SENSE_TOUCHING_MOUSE:
        case SENSE_TOUCHING_EDGE:
        case SENSE_MOUSE_DOWN:
        case SENSE_MOUSE_X:
        case SENSE_MOUSE_Y:
        case SENSE_TIMER:
        case SENSE_RESET_TIMER:
        {
            ExecutionContext ctx;
            ctx.sprite = rt->targetSprite;
            ctx.stage = stage;
            ctx.mouseX = rt->mouseX;
            ctx.mouseY = rt->mouseY;
            execute_sensing_block(b, ctx);
            rt->lastResult = ctx.lastResult;
            break;
        }

        // Operators:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_ABS:
        case OP_FLOOR:
        case OP_CEIL:
        case OP_SQRT:
        case OP_SIN:
        case OP_COS:
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_XOR:
        case OP_GT:
        case OP_LT:
        case OP_EQ:
        case OP_STR_LEN:
        case OP_STR_CHAR:
        case OP_STR_CONCAT:
        {
            ExecutionContext ctx;
            ctx.sprite = rt->targetSprite;
            ctx.stage = stage;
            ctx.mouseX = rt->mouseX;
            ctx.mouseY = rt->mouseY;
            ctx.runtime = rt;
            execute_operator_block(b, ctx);
            rt->lastResult = ctx.lastResult;
            rt->lastStringResult = ctx.lastStringResult;
            break;
        }


        // Custom Blocks:
        case CMD_DEFINE_BLOCK: {
            if (b->args.size() > 0) {
                std::string name = b->args[0];
                custom_blocks_register(name, b);
                log_info("Registered custom block: " + name);
            }
            break;
        }

        case CMD_CALL_BLOCK: {
            if (b->args.empty()) {
                log_warning("CALL_BLOCK has no function name");
                break;
            }
            
            std::string name = b->args[0];
            Block* def = custom_blocks_get(name);

            if (!def) {
                log_error("Custom block not found: " + name);
                break;
            }
                
            log_info("Calling custom block: " + name);
            
            std::vector<std::string> paramNames;
            for(size_t i=1; i<def->args.size(); i++) {
                paramNames.push_back(def->args[i]);
            }

            std::vector<std::string> values;
            for(size_t i=0; i<paramNames.size(); i++) {
                if (i < b->argBlocks.size() && b->argBlocks[i]) {
                    float val = evaluate_block_argument(rt, b, (int)i);
                        values.push_back(std::to_string(val));
                    log_debug("Arg " + std::to_string(i) + " from argBlocks: " + std::to_string(val));
                } else if (i+1 < b->args.size()) {
                    float val = resolve_argument(rt, b->args[i+1]);
                        values.push_back(std::to_string(val));
                    log_debug("Arg " + std::to_string(i) + " from args: " + std::to_string(val));
                } else {
                    values.push_back("0");
                    log_debug("Arg " + std::to_string(i) + " default: 0");
                    }
                }

                push_scope(rt, paramNames, values);

                rt->callStack.push_back(b->next);
            log_debug("Pushed to callStack, size: " + std::to_string(rt->callStack.size()));

            break;
        }
        case CMD_GOTO_RANDOM: {
            Sprite& sp = *rt->targetSprite;
            float stageLeft = (float)STAGE_X;
            float stageRight = (float)(STAGE_X + STAGE_WIDTH);
            float stageTop = (float)STAGE_Y;
            float stageBottom = (float)(STAGE_Y + STAGE_HEIGHT);

            float marginX = (sp.width * sp.scale) / 2.0f;
            float marginY = (sp.height * sp.scale) / 2.0f;

            float minX = stageLeft + marginX;
            float maxX = stageRight - marginX;
            float minY = stageTop + marginY;
            float maxY = stageBottom - marginY;

            if (maxX > minX)
                sp.x = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
            else
                sp.x = (stageLeft + stageRight) / 2.0f;

            if (maxY > minY)
                sp.y = minY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxY - minY)));
            else
                sp.y = (stageTop + stageBottom) / 2.0f;

            hasChanged = true;
            log_info("Motion: go to random (" + std::to_string((int)sp.x) + ", " + std::to_string((int)sp.y) + ")");
            break;
        }

        case CMD_GOTO_MOUSE: {
            Sprite& sp = *rt->targetSprite;
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            float oldX = sp.x;
            float oldY = sp.y;
            sp.x = (float)mx;
            sp.y = (float)my;
            hasChanged = true;

            if (sp.isPenDown && stage && stage->renderer) {
                pen_draw_line(stage->renderer, oldX, oldY, sp.x, sp.y, sp);
            }
            log_info("Motion: go to mouse (" + std::to_string(mx) + ", " + std::to_string(my) + ")");
            break;
        }
        case CMD_IF_ON_EDGE_BOUNCE: {
            Sprite& sp = *rt->targetSprite;
            float halfW = (sp.width * sp.scale) / 2.0f;
            float halfH = (sp.height * sp.scale) / 2.0f;

            float leftEdge = (float)STAGE_X;
            float rightEdge = (float)(STAGE_X + STAGE_WIDTH);
            float topEdge = (float)STAGE_Y;
            float bottomEdge = (float)(STAGE_Y + STAGE_HEIGHT);

            float rad = sp.angle * 3.14159265f / 180.0f;
            float dx = sinf(rad);
            float dy = -cosf(rad);
            bool bounced = false;

            if ((sp.x - halfW <= leftEdge && dx < 0) ||
                (sp.x + halfW >= rightEdge && dx > 0)) {
                dx = -dx;
                bounced = true;
                if (sp.x - halfW < leftEdge) sp.x = leftEdge + halfW;
                if (sp.x + halfW > rightEdge) sp.x = rightEdge - halfW;
            }

            if ((sp.y - halfH <= topEdge && dy < 0) ||
                (sp.y + halfH >= bottomEdge && dy > 0)) {
                dy = -dy;
                bounced = true;
                if (sp.y - halfH < topEdge) sp.y = topEdge + halfH;
                if (sp.y + halfH > bottomEdge) sp.y = bottomEdge - halfH;
            }

            if (bounced) {
                sp.angle = atan2f(dx, -dy) * 180.0f / 3.14159265f;
                if (sp.angle < 0) sp.angle += 360.0f;
                hasChanged = true;
                log_info("Motion: bounced, new angle = " + std::to_string((int)sp.angle));
            }
            break;
        }


    }
    if (hasChanged) {
        clamp_sprite_to_stage(*rt->targetSprite, *stage);
    }
    rt->lastExecutedBlock = b;

}

void advance_to_next_block(Runtime* rt) {
    if (!rt->currentBlock) return;

    Block* cur = rt->currentBlock;

    bool isLoopHeader = (cur->type == CMD_REPEAT || cur->type == CMD_IF);

    if (isLoopHeader && !rt->loopStack.empty() && rt->loopStack.back().loopBlock == cur) {
        if (cur->inner) {
            rt->currentBlock = cur->inner;
            return;
        } else {
            rt->loopStack.pop_back();
        }
    }

    if (cur->type == CMD_CALL_BLOCK) {
        std::string name = cur->args.empty() ? "" : cur->args[0];
        Block* def = custom_blocks_get(name);

        if (def && def->inner) {
            rt->currentBlock = def->inner;
            log_debug("Jumping to function body");
            return;
        } else {
            log_debug("Function has no body, returning immediately");
            
            if (!rt->callStack.empty()) {
                Block* returnTo = rt->callStack.back();
                rt->callStack.pop_back();
                pop_scope(rt);
                
                if (returnTo) {
                    rt->currentBlock = returnTo;
                    log_debug("Returning from function to block #" + std::to_string(returnTo->id));
                    return;
                }
            }
            rt->currentBlock = nullptr;
            return;
        }
    }

    if (cur->next) {
        rt->currentBlock = cur->next;
        return;
    }

    if (!rt->callStack.empty()) {
        Block* returnTo = rt->callStack.back();
        rt->callStack.pop_back();
        pop_scope(rt);
        
        if (returnTo) {
            rt->currentBlock = returnTo;
                log_debug("Returning from function, continuing at block #" + std::to_string(returnTo->id));
            return;
        }

        log_debug("Returning from function with null return address, checking loopStack");
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
            
            if (ctx.loopBlock->inner) {
                rt->currentBlock = ctx.loopBlock->inner;
                log_debug("REPEAT: continuing iteration " + std::to_string(ctx.remainingIterations) + " remaining");
                return;
            } else {
                rt->currentBlock = nullptr;
                return;
            }
        }

        Block* parentBlock = ctx.loopBlock;
        rt->loopStack.pop_back();
        log_debug("Popped loop context for block #" + std::to_string(parentBlock->id));

        if (parentBlock->next) {
            rt->currentBlock = parentBlock->next;
            log_debug("Continuing after loop at block #" + std::to_string(parentBlock->next->id));
            return;
        }
    }

    rt->currentBlock = nullptr;
    log_debug("No more blocks, execution finished");
}
