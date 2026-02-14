#include "block_executor_sound.h"
#include "../utils/logger.h"
#include "sound.h"

void execute_play_sound(Block* block, Sprite& sprite) {
    if (!block) return;

    std::string sound_name = "meow";
    if (!block->args.empty() && !block->args[0].empty()) {
        sound_name = block->args[0];
    }

    log_info("Playing sound " + sound_name);
    play_sound(sound_name, sprite.volume);
}

void execute_stop_all_sounds(Block* block, Sprite& sprite) {
    if (!block) return;
    log_info("Stopping all sounds");
    sound_cleanup();
}

void execute_change_volume(Block* block, Sprite& sprite) {
    if (!block) return;

    int delta = 10;
    if (!block->args.empty()) {
        delta = (int)atof(block->args[0].c_str());
    }

    sprite.volume += delta;
    if (sprite.volume < 0) sprite.volume = 0;
    if (sprite.volume > 100) sprite.volume = 100;

    set_sound_volume(sprite.volume);
    log_info("Volume changed by " + std::to_string((int)delta) + " -> " + std::to_string((int)sprite.volume));
}

void execute_set_volume(Block* block, Sprite& sprite) {
    if (!block) return;

    int vol = 100;
    if (!block->args.empty()) {
        vol = (int)atof(block->args[0].c_str());
    }

    if (vol < 0) vol = 0;
    if (vol > 100) vol = 100;

    sprite.volume = vol;
    set_sound_volume(sprite.volume);
    log_info("Volume set to " + std::to_string((int)sprite.volume));
}