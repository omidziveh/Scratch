#ifndef DRAW_H
#define DRAW_H
#include "../common/definitions.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "text_input.h"

#include "palette.h"

SDL_Texture* load_texture(SDL_Renderer* renderer, const std::string& path);

void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);
void draw_rect_outline(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color);

void draw_block(SDL_Renderer* renderer, const Block& block, const std::string& label);
void draw_all_blocks(SDL_Renderer* renderer, const std::vector<Block>& blocks);
void draw_block_glow(SDL_Renderer* renderer, const Block& block);

void draw_toolbar(SDL_Renderer* renderer);
void draw_coding_area(SDL_Renderer* renderer);
void draw_stage(SDL_Renderer* renderer, Sprite& sprite);
void draw_stage_border(SDL_Renderer* renderer);

void draw_sprite(SDL_Renderer* renderer, Sprite& sprite);

void draw_text(SDL_Renderer* renderer, int x, int y, const std::string& text, SDL_Color color);

void draw_arg_boxes(SDL_Renderer* renderer, const Block& block, const TextInputState& state);
void draw_cursor(SDL_Renderer* renderer, int x, int y, int height, SDL_Color color);

void draw_category_bar(SDL_Renderer* renderer, const std::vector<CategoryItem>& categories, int selected_index);

#endif
