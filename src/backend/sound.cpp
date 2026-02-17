#include "sound.h"
#include "../utils/logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::unordered_map<std::string, Mix_Chunk*> g_sounds;
static int g_sound_volume = 128;

static std::vector<SoundItem> g_project_sounds;
static std::vector<LibrarySound> g_library_sounds;


bool sound_init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        log_error("SDL_Mixer init failed: " + std::string(Mix_GetError()));
        return false;
    }
    Mix_AllocateChannels(16);
    log_info("Sound engine initialized");
    
    g_library_sounds.clear();
    
    auto addLib = [&](const char* n, const char* c, const char* p) {
        LibrarySound ls;
        ls.name = n;
        ls.category = c;
        ls.filepath = p;
        g_library_sounds.push_back(ls);
    };

    addLib("meow", "Animals", "../assets/sounds/library/meow.wav");
    addLib("dog bark", "Animals", "../assets/sounds/library/dog_bark.wav");
    addLib("bird", "Animals", "../assets/sounds/library/bird.wav");
    addLib("horse", "Animals", "../assets/sounds/library/horse.wav");
    addLib("pop", "Effects", "../assets/sounds/library/pop.wav");
    addLib("boing", "Effects", "../assets/sounds/library/boing.wav");
    addLib("whoosh", "Effects", "../assets/sounds/library/whoosh.wav");
    addLib("splash", "Effects", "../assets/sounds/library/splash.wav");
    addLib("laser", "Effects", "../assets/sounds/library/laser.wav");
    addLib("drum", "Music", "../assets/sounds/library/drum.wav");
    addLib("piano", "Music", "../assets/sounds/library/piano.wav");
    addLib("guitar", "Music", "../assets/sounds/library/guitar.wav");
    addLib("bell", "Music", "../assets/sounds/library/bell.wav");
    addLib("laugh", "Human", "../assets/sounds/library/laugh.wav");
    addLib("cheer", "Human", "../assets/sounds/library/cheer.wav");

    return true;
}

void sound_cleanup() {
    for (auto& pair : g_sounds) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    g_sounds.clear();
    g_project_sounds.clear();
    g_library_sounds.clear();
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

void sound_unload(const std::string& name) {
    auto it = g_sounds.find(name);
    if (it != g_sounds.end()) {
        if (it->second) {
            Mix_FreeChunk(it->second);
        }
        g_sounds.erase(it);
    }
}

void play_sound(const std::string& name, int volume) {
    auto it = g_sounds.find(name);
    if (it == g_sounds.end()) {
        log_warning("Sound not found: " + name);
        return;
    }

    if (it->second) {
        Mix_VolumeChunk(it->second, volume);
        int channel = Mix_PlayChannel(-1, it->second, 0);
        if (channel == -1) {
            log_error("Failed to play sound: " + name + " - " + Mix_GetError());
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
    if (g_sound_volume > 100) g_sound_volume = 100;
}

int get_sound_volume() {
    return g_sound_volume;
}


static std::string extract_filename(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");
    std::string filename;
    if (last_slash != std::string::npos) {
        filename = path.substr(last_slash + 1);
    } else {
        filename = path;
    }
    size_t last_dot = filename.find_last_of('.');
    if (last_dot != std::string::npos) {
        filename = filename.substr(0, last_dot);
    }
    return filename;
}

bool sound_project_add_from_file(const std::string& filepath) {
    if (filepath.empty()) return false;
    
    std::string name = extract_filename(filepath);
    
    for (const auto& s : g_project_sounds) {
        if (s.name == name) {
            log_warning("Sound already exists in project: " + name);
            return false;
        }
    }
    
    if (!sound_load(name, filepath)) {
        return false;
    }
    
    SoundItem item;
    item.name = name;
    item.filepath = filepath;
    item.loaded = true;
    g_project_sounds.push_back(item);
    
    log_info("Added sound to project: " + name);
    return true;
}

bool sound_project_add_from_library(const std::string& library_name) {
    const LibrarySound* lib_sound = nullptr;
    
    for (const auto& ls : g_library_sounds) {
        if (ls.name == library_name) {
            lib_sound = &ls;
            break;
        }
    }
    
    if (!lib_sound) return false;
    
    for (const auto& s : g_project_sounds) {
        if (s.name == library_name) {
            log_warning("Sound already in project: " + library_name);
            return false;
        }
    }
    
    if (!sound_load(lib_sound->name, lib_sound->filepath)) {
        return false;
    }
    
    SoundItem item;
    item.name = lib_sound->name;
    item.filepath = lib_sound->filepath;
    item.loaded = true;
    g_project_sounds.push_back(item);
    
    log_info("Added library sound to project: " + library_name);
    return true;
}

bool sound_project_remove(int index) {
    if (index < 0 || index >= (int)g_project_sounds.size()) return false;
    
    std::string name = g_project_sounds[index].name;
    
    sound_unload(name);
    
    g_project_sounds.erase(g_project_sounds.begin() + index);
    log_info("Removed sound: " + name);
    return true;
}

bool sound_project_remove_by_name(const std::string& name) {
    for (int i = 0; i < (int)g_project_sounds.size(); i++) {
        if (g_project_sounds[i].name == name) {
            return sound_project_remove(i);
        }
    }
    return false;
}

SoundItem* sound_project_get(int index) {
    if (index >= 0 && index < (int)g_project_sounds.size()) {
        return &g_project_sounds[index];
    }
    return nullptr;
}

SoundItem* sound_project_get_by_name(const std::string& name) {
    for (auto& s : g_project_sounds) {
        if (s.name == name) return &s;
    }
    return nullptr;
}

int sound_project_count() {
    return (int)g_project_sounds.size();
}

std::vector<std::string> sound_project_get_names() {
    std::vector<std::string> names;
    for (const auto& s : g_project_sounds) {
        names.push_back(s.name);
    }
    if (names.empty()) names.push_back("meow"); // Default fallback
    return names;
}

const std::vector<LibrarySound>& sound_library_get_all() {
    return g_library_sounds;
}

std::vector<LibrarySound> sound_library_get_by_category(const std::string& category) {
    std::vector<LibrarySound> result;
    for (const auto& ls : g_library_sounds) {
        if (category == "All" || ls.category == category) {
            result.push_back(ls);
        }
    }
    return result;
}

std::vector<std::string> sound_library_get_categories() {
    std::vector<std::string> cats;
    cats.push_back("All");
    for (const auto& ls : g_library_sounds) {
        bool found = false;
        for (const auto& c : cats) {
            if (c == ls.category) { found = true; break; }
        }
        if (!found) cats.push_back(ls.category);
    }
    return cats;
}

bool sound_project_save(const std::string& project_path) {
    std::string file = project_path + "/sounds.txt";
    std::ofstream f(file.c_str());
    if (!f.is_open()) return false;
    
    f << "SOUND_COUNT=" << g_project_sounds.size() << "\n";
    for (const auto& s : g_project_sounds) {
        f << "SOUND=" << s.name << "|" << s.filepath << "|" << s.volume << "|" << s.pitch << "\n";
    }
    return true;
}

bool sound_project_load(const std::string& project_path) {
    std::string file = project_path + "/sounds.txt";
    std::ifstream f(file.c_str());
    if (!f.is_open()) return false;
    
    for (auto& s : g_project_sounds) {
        if (s.loaded) sound_unload(s.name);
    }
    g_project_sounds.clear();
    
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        if (line.substr(0, 6) == "SOUND=") {
            std::string data = line.substr(6);
            std::stringstream ss(data);
            std::string part;
            std::vector<std::string> parts;
            while (std::getline(ss, part, '|')) parts.push_back(part);
            
            if (parts.size() >= 2) {
                std::string name = parts[0];
                std::string path = parts[1];
                if (sound_load(name, path)) {
                    SoundItem item;
                    item.name = name;
                    item.filepath = path;
                    item.loaded = true;
                    if (parts.size() >= 3) item.volume = std::stoi(parts[2]);
                    if (parts.size() >= 4) item.pitch = std::stoi(parts[3]);
                    g_project_sounds.push_back(item);
                }
            }
        }
    }
    return true;
}