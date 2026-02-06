#pragma once
#include "../common/definitions.h"
#include <vector>

namespace BlockCoding {

    Block* get_block_at(int x, int y);
    void try_snap(Block* dragged);
    void handle_mouse_down(int x, int y);
    void handle_mouse_motion(int x, int y);
    void handle_mouse_up(int x, int y);

}
