#include "input.h"
#include <vector>
using namespace std;
namespace BlockCoding {
    static Block* dragged_block = nullptr;
    static int drag_offset_x = 0;
    static int drag_offset_y = 0;
        extern vector<Block*> all_blocks;
    
    Block* get_block_at(int x, int y) {
        for (int i = all_blocks.size() - 1; i >= 0; i--) {
            Block* b = all_blocks[i];
        
            if (x >= b->x && x <= b->x + 120 &&
                y >= b->y && y <= b->y + 40) {
                return b;
            }
        }
        return nullptr;  
    }
    
    void handle_mouse_down(int x, int y) {
        Block* clicked = get_block_at(x, y);
        
        if (clicked != nullptr) {
            dragged_block = clicked;
            drag_offset_x = x - clicked->x;
            drag_offset_y = y - clicked->y;
        }
    }
    
    void handle_mouse_motion(int x, int y) {
        if (dragged_block != nullptr) {
            dragged_block->x = x - drag_offset_x;
            dragged_block->y = y - drag_offset_y;
        }
    }
    
    void handle_mouse_up(int x, int y) {
        dragged_block = nullptr;  
    }
}
