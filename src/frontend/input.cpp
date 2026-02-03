// src/frontend/input.cpp
#include "input.h"
#include <cmath>

namespace BlockCoding {
    // External reference to block list
    extern std::vector<Block*> all_blocks;
    
    // Dragging state
    static Block* dragging_block = nullptr;
    static int drag_offset_x = 0;
    static int drag_offset_y = 0;
    
    // Constants
    const int BLOCK_WIDTH = 120;
    const int BLOCK_HEIGHT = 40;
    const int SNAP_DISTANCE = 20;
    
    Block* get_block_at(int x, int y) {
        // Check from top to bottom (reverse order)
        for (int i = all_blocks.size() - 1; i >= 0; i--) {
            Block* b = all_blocks[i];
            if (x >= b->x && x <= b->x + BLOCK_WIDTH &&
                y >= b->y && y <= b->y + BLOCK_HEIGHT) {
                return b;
            }
        }
        return nullptr;
    }
    
    // NEW: Day 4 Snap Logic
    void try_snap(Block* dragged) {
        if (!dragged) return;
        
        Block* closest = nullptr;
        int min_distance = SNAP_DISTANCE + 1;
        
        // Find closest block
        for (Block* b : all_blocks) {
            if (b == dragged) continue;  // Skip self
            
            // Check horizontal alignment
            int dx = abs(b->x - dragged->x);
            if (dx > SNAP_DISTANCE) continue;
            
            // Check vertical distance (bottom of b to top of dragged)
            int bottom_of_b = b->y + BLOCK_HEIGHT;
            int dy = abs(bottom_of_b - dragged->y);
            
            if (dy < min_distance) {
                min_distance = dy;
                closest = b;
            }
        }
        
        // Snap if found
        if (closest && min_distance < SNAP_DISTANCE) {
            dragged->x = closest->x;  // Align horizontally
            dragged->y = closest->y + BLOCK_HEIGHT + 5;  // Position below (with gap)
        }
    }
    
    void handle_mouse_down(int x, int y) {
        dragging_block = get_block_at(x, y);
        if (dragging_block) {
            drag_offset_x = x - dragging_block->x;
            drag_offset_y = y - dragging_block->y;
        }
    }
    
    void handle_mouse_motion(int x, int y) {
        if (dragging_block) {
            dragging_block->x = x - drag_offset_x;
            dragging_block->y = y - drag_offset_y;
        }
    }
    
    void handle_mouse_up(int x, int y) {
        if (dragging_block) {
            // NEW: Try to snap
            try_snap(dragging_block);
            
            dragging_block = nullptr;
        }
    }
}
