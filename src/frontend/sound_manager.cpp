#include "sound_manager.h"
#include "../backend/sound.h"
#include "../utils/logger.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

SoundManagerState g_sound_manager;

static const SDL_Color SM_PANEL_BG = {45, 45, 60, 255};
static const SDL_Color SM_PANEL_BORDER = {80, 80, 100, 255};
static const SDL_Color SM_HEADER_BG = {180, 100, 200, 255};
static const SDL_Color SM_ITEM_BG = {55, 55, 75, 255};
static const SDL_Color SM_ITEM_HOVER = {70, 70, 95, 255};
static const SDL_Color SM_ITEM_SELECTED = {100, 80, 140, 255};
static const SDL_Color SM_TEXT_COLOR = {255, 255, 255, 255};
static const SDL_Color SM_BTN_ADD = {80, 160, 80, 255};
static const SDL_Color SM_BTN_DELETE = {180, 60, 60, 255};
static const SDL_Color SM_BTN_PLAY = {60, 140, 200, 255};
static const SDL_Color SM_DIALOG_BG = {35, 35, 50, 240};
static const SDL_Color SM_DIALOG_HEADER = {60, 60, 90, 255};
static const SDL_Color SM_MENU_BG = {50, 50, 70, 250};
static const SDL_Color SM_MENU_HOVER = {70, 70, 100, 255};
static const SDL_Color SM_CATEGORY_BG = {60, 60, 85, 255};
static const SDL_Color SM_CATEGORY_SELECTED = {100, 80, 160, 255};

static bool sm_point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

void sound_manager_init(SDL_Renderer* renderer, int x, int y, int w, int h) {
    g_sound_manager.renderer = renderer;
    g_sound_manager.panel_x = x;
    g_sound_manager.panel_y = y;
    g_sound_manager.panel_width = w;
    g_sound_manager.panel_height = h;
    g_sound_manager.visible = false;
    g_sound_manager.scroll_offset = 0;
    g_sound_manager.selected_index = -1;
    g_sound_manager.hover_index = -1;
    g_sound_manager.library_dialog_open = false;
    g_sound_manager.file_dialog_open = false;
    g_sound_manager.add_menu_open = false;
    g_sound_manager.library_selected_index = -1;
    g_sound_manager.selected_category = "All";
    g_sound_manager.library_scroll_offset = 0;
    
    sound_project_add_from_library("meow");
    log_info("SoundManager UI initialized");
}

void sound_manager_cleanup() {
    log_info("SoundManager UI cleaned up");
}

void sound_manager_set_visible(bool visible) { g_sound_manager.visible = visible; }
bool sound_manager_is_visible() { return g_sound_manager.visible; }

void sound_manager_open_library_dialog() {
    g_sound_manager.library_dialog_open = true;
    g_sound_manager.add_menu_open = false;
    g_sound_manager.library_selected_index = -1;
    g_sound_manager.library_scroll_offset = 0;
}

void sound_manager_open_file_dialog() {
    g_sound_manager.file_dialog_open = true;
    g_sound_manager.library_dialog_open = false;
    g_sound_manager.add_menu_open = false;
    
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char sz_file[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = sz_file;
    ofn.nMaxFile = sizeof(sz_file);
    ofn.lpstrFilter = "Audio Files\0*.WAV;*.MP3;*.OGG;*.FLAC\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        sound_project_add_from_file(std::string(ofn.lpstrFile));
    }
#endif
    g_sound_manager.file_dialog_open = false;
}

void sound_manager_close_dialogs() {
    g_sound_manager.library_dialog_open = false;
    g_sound_manager.file_dialog_open = false;
    g_sound_manager.add_menu_open = false;
}


static void sm_render_panel(SDL_Renderer* renderer) {
    int px = g_sound_manager.panel_x;
    int py = g_sound_manager.panel_y;
    int pw = g_sound_manager.panel_width;
    int ph = g_sound_manager.panel_height;
    
    roundedBoxRGBA(renderer, px, py, px + pw, py + ph, 8, SM_PANEL_BG.r, SM_PANEL_BG.g, SM_PANEL_BG.b, SM_PANEL_BG.a);
    roundedRectangleRGBA(renderer, px, py, px + pw, py + ph, 8, SM_PANEL_BORDER.r, SM_PANEL_BORDER.g, SM_PANEL_BORDER.b, 255);
    
    roundedBoxRGBA(renderer, px, py, px + pw, py + 35, 8, SM_HEADER_BG.r, SM_HEADER_BG.g, SM_HEADER_BG.b, 255);
    boxRGBA(renderer, px, py + 20, px + pw, py + 35, SM_HEADER_BG.r, SM_HEADER_BG.g, SM_HEADER_BG.b, 255);
    stringRGBA(renderer, px + 40, py + 12, "Sounds", 255, 255, 255, 255);
    
    int content_y = py + 40;
    int content_h = ph - 40 - SOUND_MANAGER_ADD_BTN_HEIGHT - 10;
    
    SDL_Rect clip_rect = {px + 5, content_y, pw - 10, content_h};
    SDL_RenderSetClipRect(renderer, &clip_rect);
    
    int item_y = content_y - g_sound_manager.scroll_offset;
    int count = sound_project_count();
    
    for (int i = 0; i < count; i++) {
        SoundItem* item = sound_project_get(i);
        if (!item) continue;
        
        if (item_y + SOUND_MANAGER_ITEM_HEIGHT > content_y && item_y < content_y + content_h) {
            int item_x = px + 8;
            int item_w = pw - 16;
            
            SDL_Color bg_color = (i == g_sound_manager.selected_index) ? SM_ITEM_SELECTED : 
                                 (i == g_sound_manager.hover_index) ? SM_ITEM_HOVER : SM_ITEM_BG;
            
            roundedBoxRGBA(renderer, item_x, item_y, item_x + item_w, item_y + SOUND_MANAGER_ITEM_HEIGHT, 5, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
            
            int play_x = item_x + 8;
            int play_y = item_y + (SOUND_MANAGER_ITEM_HEIGHT - SOUND_MANAGER_BUTTON_SIZE) / 2;
            roundedBoxRGBA(renderer, play_x, play_y, play_x + SOUND_MANAGER_BUTTON_SIZE, play_y + SOUND_MANAGER_BUTTON_SIZE, 4, SM_BTN_PLAY.r, SM_BTN_PLAY.g, SM_BTN_PLAY.b, 255);
            Sint16 tri_x[3] = {(Sint16)(play_x + 8), (Sint16)(play_x + 8), (Sint16)(play_x + 20)};
            Sint16 tri_y[3] = {(Sint16)(play_y + 6), (Sint16)(play_y + 22), (Sint16)(play_y + 14)};
            filledPolygonRGBA(renderer, tri_x, tri_y, 3, 255, 255, 255, 255);
            
            int del_x = item_x + item_w - SOUND_MANAGER_BUTTON_SIZE - 8;
            roundedBoxRGBA(renderer, del_x, play_y, del_x + SOUND_MANAGER_BUTTON_SIZE, play_y + SOUND_MANAGER_BUTTON_SIZE, 4, SM_BTN_DELETE.r, SM_BTN_DELETE.g, SM_BTN_DELETE.b, 255);
            thickLineRGBA(renderer, del_x + 8, play_y + 8, del_x + 20, play_y + 20, 2, 255, 255, 255, 255);
            thickLineRGBA(renderer, del_x + 20, play_y + 8, del_x + 8, play_y + 20, 2, 255, 255, 255, 255);
            
            stringRGBA(renderer, play_x + SOUND_MANAGER_BUTTON_SIZE + 12, item_y + SOUND_MANAGER_ITEM_HEIGHT/2 - 4, item->name.c_str(), 255, 255, 255, 255);
        }
        item_y += SOUND_MANAGER_ITEM_HEIGHT + 5;
    }
    
    SDL_RenderSetClipRect(renderer, NULL);
    
    int add_btn_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - 5;
    roundedBoxRGBA(renderer, px + 10, add_btn_y, px + pw - 10, add_btn_y + SOUND_MANAGER_ADD_BTN_HEIGHT, 6, SM_BTN_ADD.r, SM_BTN_ADD.g, SM_BTN_ADD.b, 255);
    stringRGBA(renderer, px + 35, add_btn_y + 12, "+ Add Sound", 255, 255, 255, 255);
}

static void sm_render_library_dialog(SDL_Renderer* renderer) {
    int dialog_w = SOUND_LIBRARY_DIALOG_WIDTH;
    int dialog_h = SOUND_LIBRARY_DIALOG_HEIGHT;
    int dialog_x = (WINDOW_WIDTH - dialog_w) / 2;
    int dialog_y = (WINDOW_HEIGHT - dialog_h) / 2;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 150);
    
    roundedBoxRGBA(renderer, dialog_x, dialog_y, dialog_x + dialog_w, dialog_y + dialog_h, 10, SM_DIALOG_BG.r, SM_DIALOG_BG.g, SM_DIALOG_BG.b, SM_DIALOG_BG.a);
    stringRGBA(renderer, dialog_x + 20, dialog_y + 14, "Sound Library", 255, 255, 255, 255);
    
    std::vector<std::string> categories = sound_library_get_categories();
    int cat_x = dialog_x + 10;
    int cat_y = dialog_y + 50;
    
    for (size_t i = 0; i < categories.size(); i++) {
        int btn_y = cat_y + (int)i * 35;
        SDL_Color col = (g_sound_manager.selected_category == categories[i]) ? SM_CATEGORY_SELECTED : SM_CATEGORY_BG;
        roundedBoxRGBA(renderer, cat_x, btn_y, cat_x + 100, btn_y + 30, 4, col.r, col.g, col.b, 255);
        stringRGBA(renderer, cat_x + 10, btn_y + 10, categories[i].c_str(), 255, 255, 255, 255);
    }
    
    int list_x = dialog_x + 120;
    int list_y = dialog_y + 50;
    int list_w = dialog_w - 140;
    int list_h = dialog_h - 110;
    
    roundedBoxRGBA(renderer, list_x, list_y, list_x + list_w, list_y + list_h, 6, SM_ITEM_BG.r, SM_ITEM_BG.g, SM_ITEM_BG.b, 200);
    
    std::vector<LibrarySound> filtered = sound_library_get_by_category(g_sound_manager.selected_category);
    
    SDL_Rect list_clip = {list_x, list_y, list_w, list_h};
    SDL_RenderSetClipRect(renderer, &list_clip);
    
    int item_y = list_y + 5 - g_sound_manager.library_scroll_offset;
    
    for (size_t i = 0; i < filtered.size(); i++) {
        if (item_y + 35 > list_y && item_y < list_y + list_h) {
            SDL_Color item_bg = ((int)i == g_sound_manager.library_selected_index) ? SM_ITEM_SELECTED : SM_ITEM_BG;
            roundedBoxRGBA(renderer, list_x + 5, item_y, list_x + list_w - 5, item_y + 32, 4, item_bg.r, item_bg.g, item_bg.b, 255);
            stringRGBA(renderer, list_x + 45, item_y + 10, filtered[i].name.c_str(), 255, 255, 255, 255);
        }
        item_y += 38;
    }
    
    SDL_RenderSetClipRect(renderer, NULL);
    
    int btn_y = dialog_y + dialog_h - 50;
    int add_btn_x = dialog_x + dialog_w - 110;
    
    SDL_Color add_col = (g_sound_manager.library_selected_index >= 0) ? SM_BTN_ADD : SDL_Color{60,60,60,255};
    roundedBoxRGBA(renderer, add_btn_x, btn_y, add_btn_x + 100, btn_y + 35, 5, add_col.r, add_col.g, add_col.b, 255);
    stringRGBA(renderer, add_btn_x + 35, btn_y + 12, "Add", 255, 255, 255, 255);
}

static void sm_render_add_menu(SDL_Renderer* renderer) {
    int px = g_sound_manager.panel_x;
    int py = g_sound_manager.panel_y;
    int pw = g_sound_manager.panel_width;
    int ph = g_sound_manager.panel_height;
    
    int menu_w = 180;
    int menu_h = 80;
    int menu_x = px + (pw - menu_w) / 2;
    int menu_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - menu_h - 15;
    
    roundedBoxRGBA(renderer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, 8, SM_MENU_BG.r, SM_MENU_BG.g, SM_MENU_BG.b, SM_MENU_BG.a);
    roundedRectangleRGBA(renderer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, 8, SM_PANEL_BORDER.r, SM_PANEL_BORDER.g, SM_PANEL_BORDER.b, 255);
    
    // Option 1: Choose from Library
    int opt1_y = menu_y + 8;
    if (sm_point_in_rect(g_sound_manager.hover_index, g_sound_manager.hover_index, menu_x+5, opt1_y, menu_w-10, 32)) {
         roundedBoxRGBA(renderer, menu_x+5, opt1_y, menu_x+menu_w-5, opt1_y+32, 4, SM_MENU_HOVER.r, SM_MENU_HOVER.g, SM_MENU_HOVER.b, 255);
    }
    stringRGBA(renderer, menu_x + 15, opt1_y + 10, "Choose from Library", 255, 255, 255, 255);
    
    // Option 2: Upload Sound
    int opt2_y = menu_y + 44;
    stringRGBA(renderer, menu_x + 15, opt2_y + 10, "Upload Sound", 255, 255, 255, 255);
}

void sound_manager_render() {
    if (!g_sound_manager.visible) return;
    
    SDL_Renderer* renderer = g_sound_manager.renderer;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 150);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    sm_render_panel(renderer);
    
    if (g_sound_manager.add_menu_open) {
        sm_render_add_menu(renderer);
    }
    
    if (g_sound_manager.library_dialog_open) {
        sm_render_library_dialog(renderer);
    }
}

bool sound_manager_handle_event(SDL_Event* event) {
    if (!g_sound_manager.visible) return false;
    
    int px = g_sound_manager.panel_x;
    int py = g_sound_manager.panel_y;
    int pw = g_sound_manager.panel_width;
    int ph = g_sound_manager.panel_height;
    
    if (event->type == SDL_MOUSEMOTION) {
        int mx = event->motion.x;
        int my = event->motion.y;
        
        g_sound_manager.hover_index = -1;
        
        int content_y = py + 40;
        int content_h = ph - 40 - SOUND_MANAGER_ADD_BTN_HEIGHT - 10;
        
        g_sound_manager.hover_index = -1;
        if (sm_point_in_rect(mx, my, px+8, content_y, pw-16, content_h)) {
            int item_y = content_y - g_sound_manager.scroll_offset;
            int count = sound_project_count();
            for (int i = 0; i < count; i++) {
                if (my >= item_y && my < item_y + SOUND_MANAGER_ITEM_HEIGHT) {
                    g_sound_manager.hover_index = i;
                    break;
                }
                item_y += SOUND_MANAGER_ITEM_HEIGHT + 5;
            }
        }
        return false;
    }
    
    if (event->type == SDL_MOUSEWHEEL) {
        return true;
    }
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;
        
        if (g_sound_manager.library_dialog_open) {
            int dialog_w = SOUND_LIBRARY_DIALOG_WIDTH;
            int dialog_h = SOUND_LIBRARY_DIALOG_HEIGHT;
            int dialog_x = (WINDOW_WIDTH - dialog_w) / 2;
            int dialog_y = (WINDOW_HEIGHT - dialog_h) / 2;
            
            // Click inside dialog? Handle it.
            if (sm_point_in_rect(mx, my, dialog_x, dialog_y, dialog_w, dialog_h)) {
                std::vector<std::string> categories = sound_library_get_categories();
                int cat_x = dialog_x + 10;
                int cat_y = dialog_y + 50;
                for (size_t i = 0; i < categories.size(); i++) {
                    if (sm_point_in_rect(mx, my, cat_x, cat_y + (int)i*35, 100, 30)) {
                        g_sound_manager.selected_category = categories[i];
                        g_sound_manager.library_selected_index = -1;
                        return true;
                    }
                }
            
                int list_x = dialog_x + 120;
                int list_y = dialog_y + 50;
                int list_w = dialog_w - 140;
                int list_h = dialog_h - 110;
                
                if (sm_point_in_rect(mx, my, list_x, list_y, list_w, list_h)) {
                    std::vector<LibrarySound> filtered = sound_library_get_by_category(g_sound_manager.selected_category);
                    int item_y = list_y + 5 - g_sound_manager.library_scroll_offset;
                    
                    for (size_t i = 0; i < filtered.size(); i++) {
                        if (my >= item_y && my < item_y + 32) {
                                g_sound_manager.library_selected_index = (int)i;
                            return true;
                        }
                        item_y += 38;
                    }
                }
            
                int btn_y = dialog_y + dialog_h - 50;
                int add_btn_x = dialog_x + dialog_w - 110;
                if (sm_point_in_rect(mx, my, add_btn_x, btn_y, 100, 35)) {
                    if (g_sound_manager.library_selected_index >= 0) {
                        std::vector<LibrarySound> filtered = sound_library_get_by_category(g_sound_manager.selected_category);
                        if (g_sound_manager.library_selected_index < (int)filtered.size()) {
                            sound_project_add_from_library(filtered[g_sound_manager.library_selected_index].name);
                            g_sound_manager.library_dialog_open = false;
                        }
                    }
                    return true;
                }
            }
            else {
                g_sound_manager.library_dialog_open = false;
                return true;
            }
        }
        
        if (g_sound_manager.add_menu_open) {
            int menu_w = 180;
            int menu_h = 80;
            int menu_x = px + (pw - menu_w) / 2;
            int menu_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - menu_h - 15;
            
            if (sm_point_in_rect(mx, my, menu_x, menu_y, menu_w, menu_h)) {
                if (sm_point_in_rect(mx, my, menu_x, menu_y+8, menu_w, 32)) {
                    sound_manager_open_library_dialog();
                }
                else if (sm_point_in_rect(mx, my, menu_x, menu_y+44, menu_w, 32)) {
                    sound_manager_open_file_dialog();
                }
                // Always close menu on click
                g_sound_manager.add_menu_open = false;
                return true;
            } else {
                // Click outside add menu -> close menu
                g_sound_manager.add_menu_open = false;
                return true;
            }
        }
        if (sm_point_in_rect(mx, my, px, py, pw, ph)) {
            int add_btn_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - 5;
            if (sm_point_in_rect(mx, my, px+10, add_btn_y, pw-20, SOUND_MANAGER_ADD_BTN_HEIGHT)) {
                g_sound_manager.add_menu_open = true;
                return true;
            }
            
            int content_y = py + 40;
            int content_h = ph - 40 - SOUND_MANAGER_ADD_BTN_HEIGHT - 10;
            if (sm_point_in_rect(mx, my, px+8, content_y, pw-16, content_h)) {
                int item_y = content_y - g_sound_manager.scroll_offset;
                int count = sound_project_count();
                
                for (int i = 0; i < count; i++) {
                    if (my >= item_y && my < item_y + SOUND_MANAGER_ITEM_HEIGHT) {
                        int item_x = px + 8;
                        int item_w = pw - 16;
                        int play_x = item_x + 8;
                        int play_y = item_y + (SOUND_MANAGER_ITEM_HEIGHT - SOUND_MANAGER_BUTTON_SIZE) / 2;
                        
                        if (sm_point_in_rect(mx, my, play_x, play_y, SOUND_MANAGER_BUTTON_SIZE, SOUND_MANAGER_BUTTON_SIZE)) {
                            SoundItem* s = sound_project_get(i);
                            if (s) play_sound(s->name, get_sound_volume());
                            return true;
                        }
                        
                        int del_x = item_x + item_w - SOUND_MANAGER_BUTTON_SIZE - 8;
                        if (sm_point_in_rect(mx, my, del_x, play_y, SOUND_MANAGER_BUTTON_SIZE, SOUND_MANAGER_BUTTON_SIZE)) {
                            sound_project_remove(i);
                            return true;
                        }
                        
                        g_sound_manager.selected_index = i;
                        return true;
                    }
                    item_y += SOUND_MANAGER_ITEM_HEIGHT + 5;
                }
            }
        } 
        else {
            // Click outside main panel (and not in dialogs) -> Close Panel
            g_sound_manager.visible = false;
            return true;
        }
    }
    
    return false;
}