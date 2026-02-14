#ifndef BLOCK_EXECUTOR_SOUND_H
#define BLOCK_EXECUTOR_SOUND_H

#include "../common/definitions.h"

void execute_play_sound(Block* block, Sprite& sprite);
void execute_stop_all_sounds(Block* block, Sprite& sprite);
void execute_change_volume(Block* block, Sprite& sprite);
void execute_set_volume(Block* block, Sprite& sprite);

#endif // BLOCK_EXECUTOR_SOUND_H