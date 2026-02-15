#include "input.h"
#include "block_utils.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "text_input.h"
#include "../backend/logic.h"


bool is_point_in_rect(int px, int py, float rx, float ry, float rw, float rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

static void reposition_chain(Block& head) {
    Block* current = &head;
    while (current->next) {
        Block* c = current->next;
        c->x = current->x;
        c->y = current->y + current->height;
        current = c;
    }
}

static Block* find_block_by_id(std::vector<Block>& blocks, int id) {
    for (auto& b : blocks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

static void unsnap_from_parent(std::vector<Block>& blocks, Block& block) {
    if (!block.parent) return;

    Block* p = find_block_by_id(blocks, block.parent->id);
    if (p) {
        p->next->parent = nullptr;
        p->next->parent = nullptr;
        p->next = nullptr;
        p->next = nullptr;
    }
    block.parent = nullptr;
    block.is_snapped = false;
    log_debug("Unsnapped block #" + std::to_string(block.id));
}

static void bring_to_front(std::vector<Block>& blocks, int index) {
    Block temp = blocks[index];
    blocks.erase(blocks.begin() + index);
    blocks.push_back(temp);
}

static void collect_chain_ids(std::vector<Block>& blocks, Block& head, std::vector<int>& ids) {
    ids.push_back(head.id);
    if (head.next) {
        Block* c = find_block_by_id(blocks, head.next->id);
        if (c) collect_chain_ids(blocks, *c, ids);
    }
}

void handle_mouse_down(SDL_Event& event, std::vector<Block>& blocks,
                       std::vector<PaletteItem>& palette_items,
                       int& next_block_id, int palette_scroll_offset) {
    int mx = event.button.x;
    int my = event.button.y;

    for (auto& item : palette_items) {
        float item_y = item.y - (float)palette_scroll_offset;

        if (is_point_in_rect(mx, my, item.x, item_y, item.width, item.height)) {
            Block new_block;
            new_block.id = next_block_id++;
            new_block.type = item.type;
            new_block.x = (float)item.x;
            new_block.y = (float)item_y;
            new_block.width = item.width;
            new_block.height = item.height;
            new_block.color = item.color;
            new_block.dragging = true;
            new_block.drag_offset_x = mx - item.x;
            new_block.drag_offset_y = my - item_y;

            if (new_block.type == CMD_MOVE || new_block.type == CMD_TURN ||
                new_block.type == CMD_GOTO || new_block.type == CMD_SET_X ||
                new_block.type == CMD_SET_Y || new_block.type == CMD_CHANGE_X ||
                new_block.type == CMD_CHANGE_Y) {
                new_block.args.push_back("10");
            } else if (new_block.type == CMD_REPEAT) {
                new_block.args.push_back("10");
            } else if (new_block.type == CMD_WAIT) {
                new_block.args.push_back("1");
            } else if (new_block.type == CMD_SAY) {
                new_block.args.push_back("Hello!");
            } else if (new_block.type == CMD_SET_SIZE || new_block.type == CMD_CHANGE_SIZE) {
                new_block.args.push_back("10");
            } else if (new_block.type == CMD_CHANGE_VOLUME || new_block.type == CMD_SET_VOLUME) {
                new_block.args.push_back("10");
            } else if (new_block.type == CMD_GOTO) {
                new_block.args.push_back("0");
                new_block.args.push_back("0");
            } else if (new_block.type == OP_ADD || new_block.type == OP_SUB || new_block.type == OP_DIV) {
                new_block.args.push_back("0");
                new_block.args.push_back("0");
            }

            blocks.push_back(new_block);
            log_info("Created block #" + std::to_string(new_block.id) + " from palette");
            return;
        }
    }

    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        if (is_point_in_rect(mx, my, blocks[i].x, blocks[i].y,
                             blocks[i].width, blocks[i].height)) {

            int target_id = blocks[i].id;

            unsnap_from_parent(blocks, blocks[i]);

            Block* blk = find_block_by_id(blocks, target_id);
            if (!blk) return;

            blk->dragging = true;
            blk->drag_offset_x = mx - blk->x;
            blk->drag_offset_y = my - blk->y;

            std::vector<int> chain_ids;
            collect_chain_ids(blocks, *blk, chain_ids);

            std::vector<Block> chain_blocks;
            for (int cid : chain_ids) {
                for (auto it = blocks.begin(); it != blocks.end(); ++it) {
                    if (it->id == cid) {
                        chain_blocks.push_back(*it);
                        blocks.erase(it);
                        break;
                    }
                }
            }

            for (auto& cb : chain_blocks) {
                blocks.push_back(cb);
            }

            Block* head = find_block_by_id(blocks, target_id);
            if (head) {
                head->dragging = true;
                head->drag_offset_x = mx - head->x;
                head->drag_offset_y = my - head->y;
            }

            log_debug("Started dragging block #" + std::to_string(target_id));
            return;
        }
    }
}

void handle_mouse_up(SDL_Event& event, std::vector<Block>& blocks) {
    int dragged_id = -1;

    for (auto& block : blocks) {
        if (block.dragging) {
            dragged_id = block.id;
            block.dragging = false;
            break;
        }
    }

    if (dragged_id < 0) return;

    Block* dropped = find_block_by_id(blocks, dragged_id);
    if (!dropped) return;

    if (dropped->x < PALETTE_WIDTH) {
        log_info("Block #" + std::to_string(dropped->id) + " returned to palette - deleting");

        std::vector<int> to_remove;
        collect_chain_ids(blocks, *dropped, to_remove);

        blocks.erase(
            std::remove_if(blocks.begin(), blocks.end(),
                [&to_remove](const Block& b) {
                    return std::find(to_remove.begin(), to_remove.end(), b.id) != to_remove.end();
                }),
            blocks.end());
        return;
    }

    try_snap_blocks(blocks, *dropped);

    log_debug("Dropped block #" + std::to_string(dropped->id) +
              " at (" + std::to_string((int)dropped->x) + ", " + std::to_string((int)dropped->y) + ")");
}

void handle_mouse_motion(SDL_Event& event, std::vector<Block>& blocks) {
    int mx = event.motion.x;
    int my = event.motion.y;

    for (auto& block : blocks) {
        if (!block.dragging) continue;

        float prev_x = block.x;
        float prev_y = block.y;

        block.x = mx - block.drag_offset_x;
        block.y = my - block.drag_offset_y;

        float dx = block.x - prev_x;
        float dy = block.y - prev_y;

        Block* next = block.next;
        while (next) {
            next->x += dx;
            next->y += dy;
            next = next->next;
        }
        break;
    }
}

void try_snap_blocks(std::vector<Block>& blocks, Block& dropped_block) {
    int dropped_id = dropped_block.id;
    float best_dist = (float)SNAP_DISTANCE;
    int best_target_id = -1;

    for (auto& target : blocks) {
        if (target.id == dropped_id) continue;
        if (target.next != nullptr) continue;

        float snap_x = target.x;
        float snap_y = target.y + target.height;

        float dx = std::abs(dropped_block.x - snap_x);
        float dy = std::abs(dropped_block.y - snap_y);
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < best_dist) {
            best_dist = dist;
            best_target_id = target.id;
        }
    }

    if (best_target_id < 0) return;

    Block* target = find_block_by_id(blocks, best_target_id);
    Block* dropped = find_block_by_id(blocks, dropped_id);
    if (!target || !dropped) return;

    float offset_x = target->x - dropped->x;
    float offset_y = (target->y + target->height) - dropped->y;

    dropped->x += offset_x;
    dropped->y += offset_y;
    connect_blocks(target, dropped);

    Block* ch = dropped->next;
    while (ch) {
        ch->x += offset_x;
        ch->y += offset_y;
        ch = ch->next;
    }

    log_info("Snapped block #" + std::to_string(dropped->id) +
             " to block #" + std::to_string(target->id));
}

void unsnap_block(Block& block) {
    disconnect_from_parent(&block);
    block.is_snapped = false;
    log_debug("Unsnapped block #" + std::to_string(block.id));
}
bool try_click_arg(const Block& block, int mx, int my, TextInputState& state) {
    int arg_x = (int)block.x + 80;  
    int arg_y = (int)block.y + 8;   
    int arg_w = 40;
    int arg_h = 20;
    
    for (size_t i = 0; i < block.args.size(); i++) {
        int box_x = arg_x + (int)i * 50;
        
        if (mx >= box_x && mx <= box_x + arg_w &&
            my >= arg_y && my <= arg_y + arg_h) {
            state.active = true;
            state.block_id = block.id;
            state.arg_index = (int)i;
            state.buffer = block.args[i];
            state.cursor_pos = (int)state.buffer.length();
            return true;
        }
    }
    return false;
}
