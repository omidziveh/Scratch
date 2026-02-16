#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
enum MenuAction {
    MENU_ACTION_NONE = 0,

    // File menu
    MENU_ACTION_NEW,
    MENU_ACTION_SAVE,
    MENU_ACTION_LOAD,
    MENU_ACTION_EXIT,

    // Help menu
    MENU_ACTION_SYSTEM_LOGGER,
    MENU_ACTION_DEBUG_INFO,
    MENU_ACTION_ABOUT
};

struct MenuItem {
    std::string label;
    bool        highlighted;
    MenuAction  action;     

    MenuItem() : label(""), highlighted(false), action(MENU_ACTION_NONE) {}
    MenuItem(const std::string& l) : label(l), highlighted(false), action(MENU_ACTION_NONE) {}
    MenuItem(const std::string& l, MenuAction a) : label(l), highlighted(false), action(a) {}
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

MenuAction menu_consume_action();

#endif
