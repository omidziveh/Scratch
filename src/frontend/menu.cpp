#include "menu.h"
#include "../common/definitions.h"
#include "../common/globals.h"
#include "draw.h"
#include <cstring>
static MenuAction g_last_action = MENU_ACTION_NONE;

MenuAction menu_consume_action() {
    MenuAction a = g_last_action;
    g_last_action = MENU_ACTION_NONE;
    return a;
}

static Menu g_menus[2];  
static int  g_menu_count = 2;

void menu_init() {
    g_menus[0].title  = "File";
    g_menus[0].x      = 4;
    g_menus[0].y      = 0;
    g_menus[0].width  = 60;
    g_menus[0].height = MENU_BAR_OFFSET;
    g_menus[0].is_open = false;
    g_menus[0].title_highlighted = false;
    g_menus[0].item_width  = 120;
    g_menus[0].item_height = 24;
    g_menus[0].items.clear();
    g_menus[0].items.push_back(MenuItem("New",  MENU_ACTION_NEW));    
    g_menus[0].items.push_back(MenuItem("Save", MENU_ACTION_SAVE));  
    g_menus[0].items.push_back(MenuItem("Load", MENU_ACTION_LOAD));   
    g_menus[0].items.push_back(MenuItem("Exit", MENU_ACTION_EXIT));  
    g_menus[1].title  = "Help";
    g_menus[1].x      = 62;
    g_menus[1].y      = 0;
    g_menus[1].width  = 60;
    g_menus[1].height = MENU_BAR_OFFSET;
    g_menus[1].is_open = false;
    g_menus[1].title_highlighted = false;
    g_menus[1].item_width  = 140;
    g_menus[1].item_height = 24;
    g_menus[1].items.clear();
    g_menus[1].items.push_back(MenuItem("System Logger", MENU_ACTION_SYSTEM_LOGGER)); 
    g_menus[1].items.push_back(MenuItem("Debug Info",    MENU_ACTION_DEBUG_INFO));    
    g_menus[1].items.push_back(MenuItem("About",         MENU_ACTION_ABOUT));       
}

bool menu_is_any_open() {
    for (int i = 0; i < g_menu_count; i++) {
        if (g_menus[i].is_open) return true;
    }
    return false;
}

void menu_handle_mouse_move(int mx, int my) {
    for (int i = 0; i < g_menu_count; i++) {
        Menu& m = g_menus[i];
        bool over_title = (mx >= m.x && mx <= m.x + m.width &&
                           my >= m.y && my <= m.y + m.height);
        m.title_highlighted = over_title;

        if (over_title && menu_is_any_open()) {
            for (int j = 0; j < g_menu_count; j++) {
                g_menus[j].is_open = (j == i);
            }
        }

        if (m.is_open) {
            int dropdown_x = m.x;
            int dropdown_y = m.y + m.height;

            for (int k = 0; k < (int)m.items.size(); k++) {
                int item_y = dropdown_y + k * m.item_height;
                bool over_item = (mx >= dropdown_x &&
                                  mx <= dropdown_x + m.item_width &&
                                  my >= item_y &&
                                  my <= item_y + m.item_height);
                m.items[k].highlighted = over_item;
            }
        } else {
            for (auto& item : m.items) {
                item.highlighted = false;
            }
        }
    }
}

void menu_handle_mouse_down(int mx, int my) {
    for (int i = 0; i < g_menu_count; i++) {
        Menu& m = g_menus[i];
        bool over_title = (mx >= m.x && mx <= m.x + m.width &&
                           my >= m.y && my <= m.y + m.height);
        if (over_title) {
            bool was_open = m.is_open;

            for (int j = 0; j < g_menu_count; j++) {
                g_menus[j].is_open = false;
            }

            if (!was_open) {
                m.is_open = true;
            }
            return;
        }
    }

    for (int i = 0; i < g_menu_count; i++) {
        Menu& m = g_menus[i];
        if (!m.is_open) continue;

        int dropdown_x = m.x;
        int dropdown_y = m.y + m.height;

        for (int k = 0; k < (int)m.items.size(); k++) {
            int item_y = dropdown_y + k * m.item_height;
            bool over_item = (mx >= dropdown_x &&
                              mx <= dropdown_x + m.item_width &&
                              my >= item_y &&
                              my <= item_y + m.item_height);
            if (over_item) {
                g_last_action = m.items[k].action;
                m.is_open = false;
                return;
            }
        }
    }

    for (int j = 0; j < g_menu_count; j++) {
        g_menus[j].is_open = false;
    }
}

void menu_handle_mouse_up(int mx, int my) {
    (void)mx;
    (void)my;
}


void menu_render(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 45, 45, 48, 255);
    SDL_Rect bar = { 0, 0, WINDOW_WIDTH, MENU_BAR_OFFSET };
    SDL_RenderFillRect(renderer, &bar);

    SDL_SetRenderDrawColor(renderer, 70, 70, 75, 255);
    SDL_RenderDrawLine(renderer, 0, MENU_BAR_OFFSET - 1, WINDOW_WIDTH, MENU_BAR_OFFSET - 1);

    for (int i = 0; i < g_menu_count; i++) {
        Menu& m = g_menus[i];

        if (m.title_highlighted || m.is_open) {
            SDL_SetRenderDrawColor(renderer, 70, 70, 78, 255);
            SDL_Rect hl = { m.x, m.y, m.width, m.height };
            SDL_RenderFillRect(renderer, &hl);
        }

        int text_y = (MENU_BAR_OFFSET - 14) / 2;
        draw_text(renderer, m.x + 6, text_y, m.title, COLOR_WHITE);

        if (m.is_open) {
            int dropdown_x = m.x;
            int dropdown_y = m.y + m.height;
            int dropdown_h = (int)m.items.size() * m.item_height;

            SDL_SetRenderDrawColor(renderer, 50, 50, 55, 255);
            SDL_Rect dd = { dropdown_x, dropdown_y, m.item_width, dropdown_h };
            SDL_RenderFillRect(renderer, &dd);

            SDL_SetRenderDrawColor(renderer, 80, 80, 88, 255);
            SDL_RenderDrawRect(renderer, &dd);

            for (int k = 0; k < (int)m.items.size(); k++) {
                int item_y = dropdown_y + k * m.item_height;

                if (m.items[k].highlighted) {
                    SDL_SetRenderDrawColor(renderer, 75, 110, 175, 255);
                    SDL_Rect ih = { dropdown_x + 1, item_y, m.item_width - 2, m.item_height };
                    SDL_RenderFillRect(renderer, &ih);
                }

                draw_text(renderer, dropdown_x + 10, item_y + 5, m.items[k].label, COLOR_WHITE);
            }
        }
    }
}
