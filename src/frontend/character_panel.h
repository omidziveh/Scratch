#ifndef CHARACTER_PANEL_H
#define CHARACTER_PANEL_H

#include <SDL2/SDL.h>
#include <string>
#include "../gfx/SDL2_gfxPrimitives.h"

#define CPANEL_MAX_CHARS 20
#define CPANEL_THUMB_SIZE 55

struct CharPanelItem {
    std::string name;
    SDL_Texture* thumbnail;
    bool is_visible;
    bool is_selected;
    float pos_x;
    float pos_y;
    float angle;
};

struct CharacterPanel {
    int count;
    int selected_index;
    CharPanelItem items[CPANEL_MAX_CHARS];
    bool show_add_menu;
    int panel_x;
    int panel_y;
    int panel_w;
    int panel_h;
};

void cpanel_init(CharacterPanel* panel, int px, int py, int pw, int ph);
int cpanel_add(CharacterPanel* panel, const char* name, SDL_Texture* thumb, float x, float y);
void cpanel_remove(CharacterPanel* panel, int index);
void cpanel_select(CharacterPanel* panel, int index);
void cpanel_toggle_visibility(CharacterPanel* panel, int index);
void cpanel_render(CharacterPanel* panel, SDL_Renderer* renderer);
int cpanel_handle_click(CharacterPanel* panel, int mx, int my);
int cpanel_get_selected(CharacterPanel* panel);

#endif
