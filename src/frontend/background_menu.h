#ifndef BACKGROUND_MENU_H
#define BACKGROUND_MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "../gfx/SDL2_gfxPrimitives.h"

#define MAX_BACKGROUNDS 32
#define BG_THUMB_SIZE   60
#define BG_PANEL_W      240
#define BG_PANEL_H      350

struct BackgroundItem {
    std::string name;
    std::string filepath;
    SDL_Texture* texture;
    SDL_Texture* thumbnail;
    int width;
    int height;
    bool is_solid_color;
    Uint8 solid_r, solid_g, solid_b;

    BackgroundItem()
        : texture(nullptr)
        , thumbnail(nullptr)
        , width(0)
        , height(0)
        , is_solid_color(false)
        , solid_r(255), solid_g(255), solid_b(255)
    {}
};

struct BackgroundMenu {
    bool is_open;
    int selected_index;
    int current_bg_index;
    int item_count;
    bool show_color_picker;
    Uint8 custom_r, custom_g, custom_b;
    int panel_x, panel_y;
    BackgroundItem items[MAX_BACKGROUNDS];

    BackgroundMenu()
        : is_open(false)
        , selected_index(-1)
        , current_bg_index(-1)
        , item_count(0)
        , show_color_picker(false)
        , custom_r(255), custom_g(255), custom_b(255)
        , panel_x(0), panel_y(0)
    {}
};

void bg_menu_init(BackgroundMenu* menu, int panel_x, int panel_y);
void bg_menu_destroy(BackgroundMenu* menu);

int  bg_menu_add_image(BackgroundMenu* menu, const char* name, const char* filepath, SDL_Renderer* renderer);
int  bg_menu_add_solid_color(BackgroundMenu* menu, const char* name, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer* renderer);
void bg_menu_remove_item(BackgroundMenu* menu, int index);
void bg_menu_select(BackgroundMenu* menu, int index);
void bg_menu_toggle(BackgroundMenu* menu);

void bg_render_stage_background(BackgroundMenu* menu, SDL_Renderer* renderer, int stage_x, int stage_y, int stage_w, int stage_h);
void bg_menu_render(BackgroundMenu* menu, SDL_Renderer* renderer);
int  bg_menu_handle_click(BackgroundMenu* menu, int mx, int my, SDL_Renderer* renderer);

#endif
