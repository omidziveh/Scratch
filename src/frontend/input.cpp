#include "input.h"
#include "block_utils.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include <cmath>
#include <iostream>

bool is_point_in_rect(int px, int py, float rx, float ry, float rw, float rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

void handle_mouse_down(SDL_Event& event, std::vector<Block>& blocks,
                       std::vector<PaletteItem>& palette_items,
                       int& next_block_id, int palette_scroll_offset) {
    int mx = event.button.x;
    int my = event.button.y;

    for (auto& item : palette_items) {
        float item_y = item.y - (float)palette_scroll_offset;
        
        if (mx >= item.x && mx <= item.x + item.width &&
            my >= item_y && my <= item_y + item.height) {

            Block new_block;
            new_block.id = next_block_id++;
            new_block.type = item.type;
            new_block.x = (float)mx;
            new_block.y = (float)my;
            new_block.width = item.width;
            new_block.height = item.height;
            new_block.color = item.color;
            new_block.dragging = true;
            new_block.drag_offset_x = 0;
            new_block.drag_offset_y = 0;
            new_block.args = get_default_args(item.type);
            new_block.parent = nullptr;
            new_block.child = nullptr;
            new_block.is_snapped = false;

            blocks.push_back(new_block);
            log_info("Created new block #" + std::to_string(new_block.id) +
                     " from palette item [" + item.label + "]");
            return;
        }
    }

    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        if (is_point_in_rect(mx, my, blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height)) {
            if (blocks[i].is_snapped) {
                unsnap_block(blocks[i]);
            }

            blocks[i].dragging = true;
            blocks[i].drag_offset_x = mx - blocks[i].x;
            blocks[i].drag_offset_y = my - blocks[i].y;

            Block temp = blocks[i];
            blocks.erase(blocks.begin() + i);
            blocks.push_back(temp);

            log_debug("Started dragging block #" + std::to_string(temp.id));
            return;
        }
    }
}

void handle_mouse_up(SDL_Event& event, std::vector<Block>& blocks) {
    for (auto& block : blocks) {
        if (block.dragging) {
            block.dragging = false;

            if (block.x < PALETTE_WIDTH) {
                log_info("Block #" + std::to_string(block.id) + " returned to palette — deleting");
                for (auto it = blocks.begin(); it != blocks.end(); ++it) {
                    if (it->id == block.id) {
                        blocks.erase(it);
                        break;
                    }
                }
                return;
            }

            try_snap_blocks(blocks, block);

            log_debug("Dropped block #" + std::to_string(block.id) +
                      " at (" + std::to_string((int)block.x) + ", " + std::to_string((int)block.y) + ")");
        }
    }
}

void handle_mouse_motion(SDL_Event& event, std::vector<Block>& blocks) {
    int mx = event.motion.x;
    int my = event.motion.y;

    for (auto& block : blocks) {
        if (block.dragging) {
            block.x = mx - block.drag_offset_x;
            block.y = my - block.drag_offset_y;

            Block* child = block.child;
            float offset_y = block.height;
            while (child) {
                child->x = block.x;
                child->y = block.y + offset_y;
                offset_y += child->height;
                child = child->child;
            }
            break;
        }
    }
}

void try_snap_blocks(std::vector<Block>& blocks, Block& dropped_block) {
    for (auto& target : blocks) {
        if (target.id == dropped_block.id) continue;
        if (target.child != nullptr) continue;

        float dx = std::abs(dropped_block.x - target.x);
        float dy = std::abs(dropped_block.y - (target.y + target.height));

        if (dx < SNAP_DISTANCE && dy < SNAP_DISTANCE) {
            dropped_block.x = target.x;
            dropped_block.y = target.y + target.height;
            dropped_block.is_snapped = true;
            dropped_block.parent = &target;
            target.child = &dropped_block;

            target.next = &dropped_block;
            dropped_block.next = nullptr;

            log_info("Snapped block #" + std::to_string(dropped_block.id) +
                     " to block #" + std::to_string(target.id));
            return;
        }
    }
}

void unsnap_block(Block& block) {
    if (block.parent) {
        block.parent->child = nullptr;
        block.parent->next = nullptr;
        block.parent = nullptr;
    }
    block.is_snapped = false;
    log_debug("Unsnapped block #" + std::to_string(block.id));
}
