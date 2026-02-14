#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>

struct MenuItem {
    std::string label;
    bool        highlighted;

    MenuItem() : label(""), highlighted(false) {}
    MenuItem(const std::string& l) : label(l), highlighted(false) {}
};

struct Menu {
    std::string       title;
    int               x, y, width, height;    
    bool              is_open;
    bool              title_highlighted;
    std::vector<MenuItem> items;
    int               item_width;
    int               item_height;

    Menu()
        : title("")
        , x(0), y(0), width(0), height(0)
        , is_open(false)
        , title_highlighted(false)
        , item_width(120)
        , item_height(24)
    {}
};

void menu_init();
void menu_handle_mouse_move(int mx, int my);
void menu_handle_mouse_down(int mx, int my);
void menu_handle_mouse_up(int mx, int my);
void menu_render(SDL_Renderer* renderer);
bool menu_is_any_open();

#endif
