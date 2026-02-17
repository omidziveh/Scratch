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

static Block* find_block_by_id(std::list<Block>& blocks, int id) {
    for (auto& b : blocks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

static void try_snap_to_argument(std::list<Block>& blocks, Block& dropped) {
    for (auto& target : blocks) {
        if (target.id == dropped.id) continue;

        int argCount = get_arg_count(target.type);
        for (int i = 0; i < argCount; i++) {
            SDL_Rect r = get_arg_box_rect(target, i);

            float cx = dropped.x + dropped.width / 2;
            float cy = dropped.y + dropped.height / 2;

            if (cx >= r.x && cx <= r.x + r.w && cy >= r.y && cy <= r.y + r.h) {
                
                bool compatible = (dropped.type >= OP_ADD && dropped.type <= OP_XOR) || 
                                  (dropped.type >= SENSE_TOUCHING_MOUSE && dropped.type <= SENSE_RESET_TIMER);

                if (compatible) {
                    if (target.argBlocks.size() <= (size_t)i) target.argBlocks.resize(i + 1, nullptr);
                    
                    if (target.argBlocks[i]) {
                        target.argBlocks[i]->parent = nullptr;
                    }

                    target.argBlocks[i] = &dropped;
                    dropped.parent = &target;
                    dropped.is_snapped = true;

                    dropped.x = r.x;
                    dropped.y = r.y;
                    
                    log_info("Snapped block #" + std::to_string(dropped.id) + " into arg slot " + std::to_string(i) + " of #" + std::to_string(target.id));
                    return;
                }
            }
        }
    }
}

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

static void unsnap_from_parent(std::list<Block>& blocks, Block& block) {
    if (!block.parent) return;

    Block* p = find_block_by_id(blocks, block.parent->id);
    if (p) {
        if (p->next == &block) {
        p->next = nullptr;
        } 
        if (p->inner == &block) {
            p->inner = nullptr;
        }
        for(auto& ab : p->argBlocks) {
            if(ab == &block) ab = nullptr;
        }
    }
    block.parent = nullptr;
    block.is_snapped = false;
    log_debug("Unsnapped block #" + std::to_string(block.id));
}

static void bring_to_front(std::list<Block>& blocks, Block& block) {
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (&(*it) == &block) {
            blocks.splice(blocks.end(), blocks, it);
            return;
        }
    }
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

void handle_mouse_down(SDL_Event& event, std::list<Block>& blocks,
                       std::vector<PaletteItem>& palette_items,
                       int& next_block_id, int palette_scroll_offset,
                       TextInputState& state) {
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

            auto defs = get_default_args(new_block.type);
            new_block.args = defs;

            std::string lbl = block_get_label(new_block.type);
            float min_width = lbl.size() * 8.0f + 20.0f;
            if (new_block.width < min_width) new_block.width = min_width;

            blocks.push_back(new_block);
            log_info("Created block #" + std::to_string(new_block.id) + " from palette");
            return;
        }
    }

    for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
        Block& b = *it;

        if (b.parent && b.is_snapped) {
            bool isArg = false;
            for (auto& ab : b.parent->argBlocks) {
                if (ab == &b) {
                    isArg = true;
                    break;
                }
            }
            if (isArg) {
                if (is_point_in_rect(mx, my, b.x, b.y, b.width, b.height)) {
                    for (auto& ab : b.parent->argBlocks) {
                        if (ab == &b) ab = nullptr;
                    }
                    b.parent = nullptr;
                    b.is_snapped = false;
                    
                    b.width = BLOCK_WIDTH;
                    
                    b.dragging = true;
                    b.drag_offset_x = mx - b.x;
                    b.drag_offset_y = my - b.y;
                    
                    bring_to_front(blocks, b);
                    return;
                }
            }
        }

        int headerY = (int)b.y;
        int headerH = BLOCK_HEIGHT;

        if (is_point_in_rect(mx, my, b.x, b.y, b.width, BLOCK_HEIGHT)) {
            unsnap_from_parent(blocks, b);

            b.dragging = true;
            b.drag_offset_x = mx - b.x;
            b.drag_offset_y = my - b.y;
            return;
        }
        int totalH = get_total_height(&b);
        if (is_point_in_rect(mx, my, b.x, b.y, b.width, totalH)) {
            if (try_click_arg(b, mx, my, state)) {
                return;
            }
        }
    }
}

void handle_mouse_up(SDL_Event& event, std::list<Block>& blocks) {
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

        for(auto it = blocks.begin(); it != blocks.end(); ) {
            if (ids_to_remove.count(it->id) > 0) {
                it = blocks.erase(it);
            } else {
                ++it;
            }
        }
        return;
    }

    try_snap_to_argument(blocks, *dropped);
    if (dropped->is_snapped) return;

    try_snap_blocks(blocks, *dropped);

    log_debug("Dropped block #" + std::to_string(dropped->id));
}

void handle_mouse_motion(SDL_Event& event, std::list<Block>& blocks) {
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

void try_snap_blocks(std::list<Block>& blocks, Block& dropped_block) {
    int dropped_id = dropped_block.id;
    
    dropped_block.height = get_total_height(&dropped_block);

    for (auto& target : blocks) {
        if (target.id == dropped_id) continue;
        
        target.height = get_total_height(&target);

        if ((target.type == CMD_IF || target.type == CMD_REPEAT) && !target.inner) {
            float container_height = get_total_height(&target);
            float snap_x = target.x + 15;
            float snap_y = target.y + BLOCK_HEIGHT + 5;

            bool x_aligned = std::abs(dropped_block.x - snap_x) < 60;
            
            bool y_overlapping = dropped_block.y < target.y + container_height;

            if (x_aligned && y_overlapping) {
                Block* dropped = find_block_by_id(blocks, dropped_id);
                if (!dropped) return;

                float offset_x = snap_x - dropped->x;
                float offset_y = snap_y - dropped->y;
                
                dropped->x += offset_x;
                dropped->y += offset_y;

                move_block_tree(dropped->inner, offset_x, offset_y);
                move_block_tree(dropped->next, offset_x, offset_y);

                connect_inner(&target, dropped);
                log_info("Snapped as INNER");
                return;
            }
        }

        if (!target.next) {
            float snap_x = target.x;
            float snap_y = target.y + get_total_height(&target);

            float dx = std::abs(dropped_block.x - snap_x);
            float dy = std::abs(dropped_block.y - snap_y);
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < SNAP_DISTANCE * 2) {
                Block* dropped = find_block_by_id(blocks, dropped_id);
                if (!dropped) return;

                float offset_x = snap_x - dropped->x;
                float offset_y = snap_y - dropped->y;

                dropped->x = snap_x;
                dropped->y = snap_y;

                move_block_tree(dropped->next, offset_x, offset_y);
                move_block_tree(dropped->inner, offset_x, offset_y);

                connect_blocks(&target, dropped);
                log_info("Snapped as NEXT");
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
    int count = get_arg_count(block.type);
    for (int i = 0; i < count; i++) {
        SDL_Rect r = get_arg_box_rect(block, i);
        if (r.w == 0) continue;
        if (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h) {
            if (i < (int)block.argBlocks.size() && block.argBlocks[i] != nullptr) {
                return false; 
            }
            
            state.active = true;
            state.block_id = block.id;
            state.arg_index = i;
            if (i < (int)block.args.size()) {
            state.buffer = block.args[i];
            } else {
                state.buffer = "";
            }
            state.cursor_pos = (int)state.buffer.length();
            return true;
        }
    }
    return false;
}
