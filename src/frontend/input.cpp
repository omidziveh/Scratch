#include "input.h"
#include "block_utils.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "text_input.h"
#include "../backend/logic.h"
#include "../backend/memory.h"
#include <set>


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

static void collect_chain_ids_recursive(Block* head, std::set<int>& ids) {
    if(!head) return;

    ids.insert(head->id);
    collect_chain_ids_recursive(head->next, ids);
    collect_chain_ids_recursive(head->inner, ids);
}

static void move_block_tree(Block* head, float dx, float dy) {
    if (!head) return;
    head->x += dx;
    head->y += dy;
    move_block_tree(head->next, dx, dy);
    move_block_tree(head->inner, dx, dy);
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
                new_block.color = COLOR_GREEN;
            }

            blocks.push_back(new_block);
            log_info("Created block #" + std::to_string(new_block.id) + " from palette");
            return;
        }
    }

    for (int i = (int)blocks.size() - 1; i >= 0; i--) {
        Block& b = blocks[i];

        b.height = get_total_height(&b);

        if (is_point_in_rect(mx, my, b.x, b.y, b.width, b.height)) {
            int target_id = b.id;

            unsnap_from_parent(blocks, b);

            Block* blk = find_block_by_id(blocks, target_id);
            if (!blk) return;

            blk->dragging = true;
            blk->drag_offset_x = mx - blk->x;
            blk->drag_offset_y = my - blk->y;

            std::vector<int> ids;
            ids.push_back(blk->id);
            
            Block temp = *blk;
            blk->dragging = true; 
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
        std::set<int> ids_to_remove;

        collect_chain_ids_recursive(dropped, ids_to_remove);

        auto it = std::remove_if(blocks.begin(), blocks.end(), 
            [&ids_to_remove](const Block& b) {
                return ids_to_remove.count(b.id) > 0;
            });
        
        blocks.erase(it, blocks.end());
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

        move_block_tree(block.inner, dx, dy);
        move_block_tree(block.next, dx, dy);
        break;
    }
}

void try_snap_blocks(std::vector<Block>& blocks, Block& dropped_block) {
    int dropped_id = dropped_block.id;
    
    dropped_block.height = get_total_height(&dropped_block);

    for (auto& target : blocks) {
        if (target.id == dropped_id) continue;
        
        target.height = get_total_height(&target);

        // NEXT:
        if (!target.next) {
            float snap_x = target.x;
            float snap_y = target.y + get_total_height(&target);

            float dx = std::abs(dropped_block.x - snap_x);
            float dy = std::abs(dropped_block.y - snap_y);
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < SNAP_DISTANCE * 2) {
                Block* dropped = find_block_by_id(blocks, dropped_id);
                if (!dropped) return;

                dropped->x = snap_x;
                dropped->y = snap_y;
                connect_blocks(&target, dropped);
                log_info("Snapped as NEXT");
                return;
            }
        }

        // INNER:
        if ((target.type == CMD_IF || target.type == CMD_REPEAT) && !target.inner) {
            float snap_x = target.x + 20;
            float snap_y = target.y + BLOCK_HEIGHT;

            float dx = std::abs(dropped_block.x - snap_x);
            float dy = std::abs(dropped_block.y - snap_y);
            bool x_match = std::abs(dropped_block.x - (target.x + 20)) < 30;
            bool y_match = (dropped_block.y > target.y + BLOCK_HEIGHT - 10) && 
                           (dropped_block.y < target.y + get_total_height(&target));

            if (x_match && y_match) {
                Block* dropped = find_block_by_id(blocks, dropped_id);
                if (!dropped) return;

                float offset_x = (target.x + 20) - dropped->x;
                float offset_y = (target.y + BLOCK_HEIGHT) - dropped->y;
                
                dropped->x += offset_x;
                dropped->y += offset_y;

                move_block_tree(dropped->inner, offset_x, offset_y);
                move_block_tree(dropped->next, offset_x, offset_y);

                connect_inner(&target, dropped);
                log_info("Snapped as INNER");
                return;
            }
        }
    }
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
