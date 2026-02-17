#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H
#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <list>


void begin_editing(TextInputState& state, Block& block, int arg_index);

void commit_editing(TextInputState& state, std::list<Block>& blocks);

void cancel_editing(TextInputState& state);

void on_text_input(TextInputState& state, const char* text);

void on_key_input(TextInputState& state, SDL_Keycode key, std::list<Block>& blocks);

void tick_cursor(TextInputState& state);

#endif
