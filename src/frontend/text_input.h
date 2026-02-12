#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include "../common/definitions.h"

int get_arg_count(BlockType type);

SDL_Rect get_arg_box_rect(const Block& block, int arg_index);

int try_click_arg(const Block& block, int mx, int my);

void begin_editing(TextInputState& state, Block& block, int arg_index);

void commit_editing(TextInputState& state, std::vector<Block>& blocks);

void cancel_editing(TextInputState& state);

void on_text_input(TextInputState& state, const char* text);

void on_key_input(TextInputState& state, SDL_Keycode key, std::vector<Block>& blocks);

void tick_cursor(TextInputState& state);

#endif
