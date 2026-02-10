#include "block_executor_sensing.h"
#include "sensing.h"
#include "operators.h"
#include "logger.h"
#include <string>
#include <cstdlib>

bool execute_sensing_block(Block* block, ExecutionContext& ctx) {
    if (!block || !ctx.sprite || !ctx.stage) return false;

    switch (block->type) {
        case SENSE_TOUCHING_MOUSE: {
            ctx.lastCondition = is_sprite_touching_mouse(*ctx.sprite, *ctx.stage, ctx.mouseX, ctx.mouseY);
            log_info("Sensing: touching mouse = " + std::string(ctx.lastCondition ? "true" : "false"));
            return true;
        }
        case SENSE_TOUCHING_EDGE: {
            ctx.lastCondition = is_sprite_touching_edge(*ctx.sprite, *ctx.stage);
            if (ctx.lastCondition) {
                bounce_off_edge(*ctx.sprite, *ctx.stage);
                log_info("Sensing: sprite hit edge, bounced");
            }
            return true;
        }
        default:
            return false;
    }
}

bool execute_operator_block(Block* block, ExecutionContext& ctx) {
    if (!block) return false;

    float a = 0.0f;
    float b = 0.0f;

    if (block->args.size() >= 1) a = std::atof(block->args[0].c_str());
    if (block->args.size() >= 2) b = std::atof(block->args[1].c_str());

    bool success = true;

    switch (block->type) {
        case OP_ADD: {
            ctx.lastResult = op_add(a, b);
            log_info("Operator: " + std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(ctx.lastResult));
            return true;
        }
        case OP_SUB: {
            ctx.lastResult = op_sub(a, b);
            log_info("Operator: " + std::to_string(a) + " - " + std::to_string(b) + " = " + std::to_string(ctx.lastResult));
            return true;
        }
        case OP_DIV: {
            ctx.lastResult = op_div(a, b, success);
            if (!success) {
                log_error("Operator: division by zero blocked");
                ctx.lastResult = 0.0f;
            } else {
                log_info("Operator: " + std::to_string(a) + " / " + std::to_string(b) + " = " + std::to_string(ctx.lastResult));
            }
            return true;
        }
        default:
            return false;
    }
}
