#ifndef INPUT_H
#define INPUT_H
#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>

#include "text_input.h" 
bool is_point_in_rect(int px, int py, float rx, float ry, float rw, float rh);

void handle_mouse_down(SDL_Event& event, std::vector<Block>& blocks,
                       std::vector<PaletteItem>& palette_items,
                       int& next_block_id, int palette_scroll_offset);

void handle_mouse_up(SDL_Event& event, std::vector<Block>& blocks);
void handle_mouse_motion(SDL_Event& event, std::vector<Block>& blocks);
void try_snap_blocks(std::vector<Block>& blocks, Block& dropped_block);
void unsnap_block(Block& block);
bool try_click_arg(const Block& block, int mx, int my, TextInputState& state);

#endif
