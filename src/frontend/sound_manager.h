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

struct SoundItem;
struct LibrarySound;

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
};

extern SoundManagerState g_sound_manager;

void sound_manager_init(SDL_Renderer* renderer, int x, int y, int w, int h);
void sound_manager_cleanup();
void sound_manager_render();
bool sound_manager_handle_event(SDL_Event* event);

void sound_manager_set_visible(bool visible);
bool sound_manager_is_visible();
void sound_manager_open_library_dialog();
void sound_manager_open_file_dialog();
void sound_manager_close_dialogs();

#endif