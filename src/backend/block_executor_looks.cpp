#include "block_executor_looks.h"
#include "../utils/logger.h"
#include <cstdlib>
#include <algorithm>
#include <string>

bool execute_looks_block(Block* block, ExecutionContext& ctx) {
    if (!block || !ctx.sprite) return false;

    Sprite& sp = *ctx.sprite;

    switch (block->type) {

        case CMD_SWITCH_COSTUME: {
            if (sp.costumes.empty()) return false;

            int idx = 0;
            if (!block->args.empty())
                idx = std::atoi(block->args[0].c_str()) - 1;

            if (idx < 0) idx = 0;
            idx = idx % (int)sp.costumes.size();

            sp.currentCostumeIndex = idx;
            sp.texture = sp.costumes[idx].texture;
            sp.width   = sp.costumes[idx].width;
            sp.height  = sp.costumes[idx].height;

            log_info("Looks: switched to costume " + std::to_string(idx + 1)
                     + " (" + sp.costumes[idx].name + ")");
            return true;
        }

        case CMD_NEXT_COSTUME: {
            if (sp.costumes.empty()) return false;

            sp.currentCostumeIndex = (sp.currentCostumeIndex + 1) % (int)sp.costumes.size();
            int idx = sp.currentCostumeIndex;

            sp.texture = sp.costumes[idx].texture;
            sp.width   = sp.costumes[idx].width;
            sp.height  = sp.costumes[idx].height;

            log_info("Looks: next costume -> " + std::to_string(idx + 1)
                     + " (" + sp.costumes[idx].name + ")");
            return true;
        }

        case CMD_SET_SIZE: {
            float pct = 100.0f;
            if (!block->args.empty())
                pct = (float)std::atof(block->args[0].c_str());

            pct = std::max(5.0f, std::min(500.0f, pct));
            sp.scale = pct / 100.0f;

            log_info("Looks: set size to " + std::to_string((int)pct) + "%");
            return true;
        }

        case CMD_CHANGE_SIZE: {
            float delta = 10.0f;
            if (!block->args.empty())
                delta = (float)std::atof(block->args[0].c_str());

            float newPct = (sp.scale * 100.0f) + delta;
            newPct = std::max(5.0f, std::min(500.0f, newPct));
            sp.scale = newPct / 100.0f;

            log_info("Looks: change size by " + std::to_string((int)delta)
                     + " -> " + std::to_string((int)newPct) + "%");
            return true;
        }

        case CMD_SHOW: {
            sp.visible = 1;
            log_info("Looks: show sprite");
            return true;
        }

        case CMD_HIDE: {
            sp.visible = 0;
            log_info("Looks: hide sprite");
            return true;
        }

        default:
            return false;
    }
}
