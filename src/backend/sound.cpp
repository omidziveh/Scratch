#include "sound.h"
#include "../utils/logger.h"
#include <unordered_map>

std::unordered_map<std::string, Mix_Chunk*> g_sounds;
static int g_sound_volume = 128;

bool sound_init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        log_error("SDL_Mixer init failed: " + std::string(Mix_GetError()));
        return false;
    }
    Mix_AllocateChannels(16);
    log_info("Sound engine initialized");
    return true;
}

void sound_cleanup() {
    for (auto& pair : g_sounds) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    g_sounds.clear();
    Mix_Quit();
    log_info("Sound engine cleaned up");
}

bool sound_load(const std::string& name, const std::string& path) {
    if (g_sounds.find(name) != g_sounds.end()) {
        Mix_FreeChunk(g_sounds[name]);
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        log_error("Failed to load sound: " + name + " - " + Mix_GetError());
        return false;
    }

    g_sounds[name] = chunk;
    log_info("Loaded sound: " + name);
    return true;
}

void play_sound(const std::string& name) {
    auto it = g_sounds.find(name);
    if (it == g_sounds.end()) {
        log_warning("Sound not found: " + name);
        return;
    }

    if (it->second) {
        Mix_VolumeChunk(it->second, g_sound_volume);
        int channel = Mix_PlayChannel(-1, it->second, 0);
        if (channel == -1) {
            log_error("Failed to play sound: " + name + " - " + Mix_GetError());
        } else {
            log_info("Playing sound: " + name);
        }
    }
}

void stop_all_sounds() {
    Mix_HaltChannel(-1);
    log_info("All sounds stopped");
}

void set_sound_volume(int volume) {
    g_sound_volume = volume;
    if (g_sound_volume < 0) g_sound_volume = 0;
    if (g_sound_volume > 128) g_sound_volume = 128;
}
