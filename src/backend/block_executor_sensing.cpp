#include "block_executor_sensing.h"
#include "sensing.h"
#include "operators.h"
#include "runtime.h"
#include "../utils/logger.h"
#include "../common/definitions.h"
#include <string>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>

extern float g_timer_value;
static Uint32 g_timer_start_time = 0;

bool execute_sensing_block(Block* block, ExecutionContext& ctx) {
    if (!block || !ctx.sprite || !ctx.stage) return false;

    switch (block->type) {
        case SENSE_TOUCHING_MOUSE: {
            ctx.lastCondition = is_sprite_touching_mouse(*ctx.sprite, *ctx.stage, ctx.mouseX, ctx.mouseY);
            ctx.lastResult = ctx.lastCondition ? 1.0f : 0.0f;
            log_info("Sensing: touching mouse = " + std::string(ctx.lastCondition ? "true" : "false"));
            return true;
        }
        case SENSE_TOUCHING_EDGE: {
            ctx.lastCondition = is_sprite_touching_edge(*ctx.sprite, *ctx.stage);
            ctx.lastResult = ctx.lastCondition ? 1.0f : 0.0f;
            return true;
        }
        case SENSE_MOUSE_DOWN: {
            int mx, my;
            Uint32 buttons = SDL_GetMouseState(&mx, &my);
            ctx.lastCondition = (buttons & SDL_BUTTON_LMASK) != 0;
            ctx.lastResult = ctx.lastCondition ? 1.0f : 0.0f;
            return true;
        }
        case SENSE_MOUSE_X: {
            ctx.lastResult = (float)(ctx.mouseX - (STAGE_X + STAGE_WIDTH / 2)); // Relative to center
            return true;
        }
        case SENSE_MOUSE_Y: {
            ctx.lastResult = (float)( (STAGE_Y + STAGE_HEIGHT / 2) - ctx.mouseY );
            return true;
        }
        case SENSE_TIMER: {
            Uint32 now = SDL_GetTicks();
            ctx.lastResult = (float)(now - g_timer_start_time) / 1000.0f;
            return true;
        }
        case SENSE_RESET_TIMER: {
            g_timer_start_time = SDL_GetTicks();
            log_info("Timer reset");
            return true;
        }
        case SENSE_DISTANCE_TO_MOUSE: {
            float spriteX = ctx.sprite->x;
            float spriteY = ctx.sprite->y;
            float mouseX = (float)ctx.mouseX;
            float mouseY = (float)ctx.mouseY;
            float ddx = spriteX - mouseX;
            float ddy = spriteY - mouseY;
            ctx.lastResult = sqrt(ddx * ddx + ddy * ddy);
            log_info("Sensing: distance to mouse = " + std::to_string((int)ctx.lastResult));
            return true;
        }

        default:
            return false;
    }
}

bool execute_operator_block(Block* block, ExecutionContext& ctx) {
    if (!block) return false;

    auto getFloat = [&](int idx) -> float {
        if (idx < (int)block->argBlocks.size() && block->argBlocks[idx] != nullptr) {
            if (ctx.runtime) {
                Block* subBlock = block->argBlocks[idx];
                execute_block(ctx.runtime, subBlock, ctx.stage);
                float result = ctx.lastResult;
                
                subBlock->is_running = false;
                
                return result;
            }
            return 0.0f;
        }

        if (idx < (int)block->args.size()) {
            if (ctx.runtime) {
                 return resolve_argument(ctx.runtime, block->args[idx]);
            }
            try { 
                return std::stof(block->args[idx]); 
            } catch (...) { 
                if (ctx.runtime && ctx.runtime->targetSprite) {
                    ctx.runtime->targetSprite->sayText = "Error: Invalid number '" + block->args[idx] + "'";
                    ctx.runtime->targetSprite->sayStartTime = SDL_GetTicks();
                }
                log_error("Invalid numeric input: " + block->args[idx]);
                return 0.0f; 
            }

        }
        return 0.0f;
    };

    auto getString = [&](int idx) -> std::string {
        if (idx < (int)block->argBlocks.size() && block->argBlocks[idx] != nullptr) {
            if (ctx.runtime) {
                Block* subBlock = block->argBlocks[idx];
                execute_block(ctx.runtime, subBlock, ctx.stage);
                
                std::string result;
                if (!ctx.lastStringResult.empty()) result = ctx.lastStringResult;
                else result = std::to_string(ctx.lastResult);

                subBlock->is_running = false;
                
                return result;
            }
            return "";
        }

        if (idx < (int)block->args.size()) {
            if (ctx.runtime) {
                return resolve_string_variable(ctx.runtime, block->args[idx]);
            }
            return block->args[idx];
        }
        return "";
    };

    bool success = true;

    switch (block->type) {
        // Math Binary
        case OP_ADD: ctx.lastResult = op_add(getFloat(0), getFloat(1)); return true;
        case OP_SUB: ctx.lastResult = op_sub(getFloat(0), getFloat(1)); return true;
        case OP_MUL: ctx.lastResult = op_mul(getFloat(0), getFloat(1)); return true;
        case OP_DIV: {
            float a = getFloat(0);
            float b = getFloat(1);
            ctx.lastResult = op_div(a, b, success);
            if (!success && ctx.runtime && ctx.runtime->targetSprite) {
                ctx.runtime->targetSprite->sayText = "Error! Division by zero";
                ctx.runtime->targetSprite->sayStartTime = SDL_GetTicks();
                ctx.runtime->targetSprite->sayDuration = 3000;
                log_error("Division by zero");
            }
            return true;
        }

        case OP_MOD: {
            float a = getFloat(0);
            float b = getFloat(1);
            ctx.lastResult = op_mod(a, b, success);
            if (!success && ctx.runtime && ctx.runtime->targetSprite) {
                ctx.runtime->targetSprite->sayText = "Error! Division by zero";
                ctx.runtime->targetSprite->sayStartTime = SDL_GetTicks();
                ctx.runtime->targetSprite->sayDuration = 3000;
                log_error("Mod by zero");
            }
            return true;
        }
        case OP_ABS:   ctx.lastResult = op_abs(getFloat(0)); return true;
        case OP_FLOOR: ctx.lastResult = op_floor(getFloat(0)); return true;
        case OP_CEIL:  ctx.lastResult = op_ceil(getFloat(0)); return true;
        case OP_SQRT: {
            float a = getFloat(0);
            ctx.lastResult = op_sqrt(a, success);
            if (!success && ctx.runtime && ctx.runtime->targetSprite) {
                ctx.runtime->targetSprite->sayText = "Error! Sqrt of negative";
                ctx.runtime->targetSprite->sayStartTime = SDL_GetTicks();
                ctx.runtime->targetSprite->sayDuration = 3000;
                log_error("Sqrt of negative");
            }
            return true;
        }

        case OP_SIN:   ctx.lastResult = op_sin(getFloat(0)); return true;
        case OP_COS:   ctx.lastResult = op_cos(getFloat(0)); return true;

        // Logic
        case OP_AND: ctx.lastResult = op_and(getFloat(0), getFloat(1)); return true;
        case OP_OR:  ctx.lastResult = op_or(getFloat(0), getFloat(1)); return true;
        case OP_NOT: ctx.lastResult = op_not(getFloat(0)); return true;
        case OP_XOR: ctx.lastResult = op_xor(getFloat(0), getFloat(1)); return true;

        // Comparison
        case OP_GT: ctx.lastResult = op_gt(getFloat(0), getFloat(1)); return true;
        case OP_LT: ctx.lastResult = op_lt(getFloat(0), getFloat(1)); return true;
        case OP_EQ: ctx.lastResult = op_eq(getFloat(0), getFloat(1)); return true;

        // String
        case OP_STR_LEN: {
            ctx.lastResult = op_str_len(getString(0));
            return true;
        }
        case OP_STR_CHAR: {
            ctx.lastStringResult = op_str_char(getString(0), getFloat(1));
            ctx.lastResult = 0.0f ;
            return true;
        }
        case OP_STR_CONCAT: {
            ctx.lastStringResult = op_str_concat(getString(0), getString(1));
            ctx.lastResult = 0.0f ;
            return true;
        }
        
        default:
            return false;
    }
}
