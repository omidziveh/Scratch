#ifndef INPUT_H
#define INPUT_H

#include "../common/definitions.h"
#include <vector>

Block* spawn_block_from_palette(BlockType type, int x, int y);
Block* get_block_at(int x, int y);
void try_snap(Block* dragged);
void handle_mouse_down(int x, int y);
void handle_mouse_motion(int x, int y);
void handle_mouse_up(int x, int y);

#endif
