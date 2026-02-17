#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#define SOUND_MANAGER_ITEM_HEIGHT 45
#define SOUND_MANAGER_BUTTON_SIZE 28
#define SOUND_MANAGER_ADD_BTN_HEIGHT 40
#define SOUND_LIBRARY_DIALOG_WIDTH 500
#define SOUND_LIBRARY_DIALOG_HEIGHT 400
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

struct SoundItem {
    std::string name;
    std::string filepath;
    bool loaded;
    float duration;
    int volume;
    int pitch;
};

struct LibrarySound {
    std::string name;
    std::string category;
    std::string filepath;
};

struct SoundManagerState {
    SDL_Renderer* renderer;
    int panel_x;
    int panel_y;
    int panel_width;
    int panel_height;
    bool visible;
    int scroll_offset;
    int selected_index;
    int hover_index;
    bool library_dialog_open;
    bool file_dialog_open;
    bool add_menu_open;
    int library_selected_index;
    std::string selected_category;
    int library_scroll_offset;
    std::vector<SoundItem> sounds;
    std::vector<LibrarySound> library;
};

extern SoundManagerState g_sound_manager;

void sound_manager_init(SDL_Renderer* renderer, int x, int y, int w, int h);
void sound_manager_cleanup();
void sound_manager_render();
bool sound_manager_handle_event(SDL_Event* event);

bool sound_manager_add_from_file(const std::string& filepath);
bool sound_manager_add_from_library(const std::string& library_name);
bool sound_manager_remove(int index);
bool sound_manager_remove_by_name(const std::string& name);

SoundItem* sound_manager_get(int index);
SoundItem* sound_manager_get_by_name(const std::string& name);
int sound_manager_count();
std::vector<std::string> sound_manager_get_names();

std::vector<LibrarySound> sound_manager_get_library_by_category(const std::string& category);
std::vector<std::string> sound_manager_get_categories();

void sound_manager_preview(const std::string& name);
void sound_manager_stop_preview();

bool sound_manager_save(const std::string& project_path);
bool sound_manager_load(const std::string& project_path);

void sound_manager_set_visible(bool visible);
bool sound_manager_is_visible();

void sound_manager_open_library_dialog();
void sound_manager_open_file_dialog();
void sound_manager_close_dialogs();
void sound_unload(const std::string& name);

#endif
