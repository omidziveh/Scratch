#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <unordered_map>

#ifdef __linux__
#include <SDL2/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

extern std::unordered_map<std::string, Mix_Chunk*> g_sounds;

bool sound_init();
void sound_cleanup();
bool sound_load(const std::string& name, const std::string& path);
void play_sound(const std::string& name, int volume);
void stop_all_sounds();
void set_sound_volume(int volume);
int get_sound_volume();

#endif