// src/frontend/input.h
#ifndef INPUT_H
#define INPUT_H

#include "../common/definitions.h"

namespace BlockCoding {
    // Mouse handlers
    void handle_mouse_down(int x, int y);
    void handle_mouse_motion(int x, int y);
    void handle_mouse_up(int x, int y);
    
    // Helper functions
    Block* get_block_at(int x, int y);
    
    // NEW: Snap logic (Day 4)
    void try_snap(Block* dragged);
}

#endif
