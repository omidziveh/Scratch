#ifndef SENSING_H
#define SENSING_H

#include "../common/definitions.h"

bool is_sprite_touching_mouse(const Sprite& sprite, const Stage& stage, int mouseX, int mouseY);
bool is_sprite_touching_edge(const Sprite& sprite, const Stage& stage);
bool is_sprite_touching_left_edge(const Sprite& sprite, const Stage& stage);
bool is_sprite_touching_right_edge(const Sprite& sprite, const Stage& stage);
bool is_sprite_touching_top_edge(const Sprite& sprite, const Stage& stage);
bool is_sprite_touching_bottom_edge(const Sprite& sprite, const Stage& stage);
void bounce_off_edge(Sprite& sprite, const Stage& stage);
void clamp_sprite_to_stage(Sprite& sprite, const Stage& stage);

#endif
