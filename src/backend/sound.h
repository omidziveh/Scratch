#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <vector>
#include <unordered_map>
#include "../common/definitions.h" 
#ifdef __linux__
#include <SDL2/SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

struct SoundItem {
    std::string name;
    std::string filepath;
    bool loaded;
    float duration;
    int volume;
    int pitch;

    SoundItem() : loaded(false), duration(0.0f), volume(100), pitch(0) {}
};

struct LibrarySound {
    std::string name;
    std::string category;
    std::string filepath;
};

extern std::unordered_map<std::string, Mix_Chunk*> g_sounds;

bool sound_init();
void sound_cleanup();
bool sound_load(const std::string& name, const std::string& path);
void sound_unload(const std::string& name); // Added specific unload
void play_sound(const std::string& name, int volume);
void stop_all_sounds();
void set_sound_volume(int volume);
int get_sound_volume();

bool sound_project_add_from_file(const std::string& filepath);
bool sound_project_add_from_library(const std::string& library_name);
bool sound_project_remove(int index);
bool sound_project_remove_by_name(const std::string& name);

SoundItem* sound_project_get(int index);
SoundItem* sound_project_get_by_name(const std::string& name);
int sound_project_count();
std::vector<std::string> sound_project_get_names();

const std::vector<LibrarySound>& sound_library_get_all();
std::vector<LibrarySound> sound_library_get_by_category(const std::string& category);
std::vector<std::string> sound_library_get_categories();

bool sound_project_save(const std::string& project_path);
bool sound_project_load(const std::string& project_path);

#endif