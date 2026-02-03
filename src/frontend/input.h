#ifndef INPUT_H
#define INPUT_H

#include "../common/definitions.h"

namespace BlockCoding {
    Block* get_block_at(int x, int y);
    void handle_mouse_down(int x, int y);
    void handle_mouse_motion(int x, int y);
    void handle_mouse_up(int x, int y);
}

#endif
