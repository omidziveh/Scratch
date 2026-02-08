#include "input.h"
#include "palette.h"
#include <cmath>

namespace BlockCoding {

    static std::vector<Block*> all_blocks;

    static Block* dragging_block = nullptr;
    static int drag_offset_x = 0;
    static int drag_offset_y = 0;
    static const int SNAP_DISTANCE = 20;

    Block* get_block_at(int x, int y) {
        for (int i = (int)all_blocks.size() - 1; i >= 0; i--) {
            Block* b = all_blocks[i];
            if (x >= b->x && x <= b->x + BLOCK_WIDTH &&
                y >= b->y && y <= b->y + BLOCK_HEIGHT) {
                return b;
            }
        }
        return nullptr;
    }

    void try_snap(Block* dragged) {
        if (!dragged) return;

        Block* closest = nullptr;
        float min_dist = SNAP_DISTANCE + 1;

        for (Block* b : all_blocks) {
            if (b == dragged) continue;

            float dx = std::abs(b->x - dragged->x);
            if (dx > SNAP_DISTANCE) continue;

            float dy = dragged->y - (b->y + BLOCK_HEIGHT);
            if (dy >= 0 && dy < min_dist) {
                min_dist = dy;
                closest = b;
            }
        }

        if (closest) {
            dragged->x = closest->x;
            dragged->y = closest->y + BLOCK_HEIGHT;
            closest->next = dragged;
        }
    }

    void handle_mouse_down(int x, int y) {
        Block* clicked = get_block_at(x, y);
        if (clicked) {
            dragging_block = clicked;
            drag_offset_x = x - (int)clicked->x;
            drag_offset_y = y - (int)clicked->y;

            for (Block* b : all_blocks) {
                if (b->next == clicked) {
                    b->next = nullptr;
                }
            }
        }
    }

    void handle_mouse_motion(int x, int y) {
        if (dragging_block) {
            dragging_block->x = (float)(x - drag_offset_x);
            dragging_block->y = (float)(y - drag_offset_y);
        }
    }

    void handle_mouse_up(int x, int y) {
        if (dragging_block) {
            try_snap(dragging_block);
            dragging_block = nullptr;
        }
    }

}
extern std::vector<Block*> all_blocks;
static int next_block_id = 100;

Block* spawn_block_from_palette(BlockType type, int x, int y) {
    Block* newBlock = new Block;
    newBlock->id = next_block_id++;
    newBlock->type = type;
    newBlock->x = x;
    newBlock->y = y;
    newBlock->width = 120;
    newBlock->height = 50;
    newBlock->next = nullptr;
    newBlock->inner = nullptr;
    
    all_blocks.push_back(newBlock);
    return newBlock;
}

