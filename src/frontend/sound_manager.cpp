#include "sound_manager.h"
#include "../utils/logger.h"
#include "../backend/sound.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <algorithm>
#include <fstream>
#include <sstream>

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
static const SDL_Color SM_TEXT_DIM = {180, 180, 180, 255};
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

static std::string sm_extract_filename(const std::string& path) {
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

static void sm_init_library() {
    g_sound_manager.library.clear();
    
    LibrarySound ls;
    
    ls.name = "meow";
    ls.category = "Animals";
    ls.filepath = "assets/sounds/library/meow.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "dog bark";
    ls.category = "Animals";
    ls.filepath = "assets/sounds/library/dog_bark.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "bird";
    ls.category = "Animals";
    ls.filepath = "assets/sounds/library/bird.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "horse";
    ls.category = "Animals";
    ls.filepath = "assets/sounds/library/horse.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "pop";
    ls.category = "Effects";
    ls.filepath = "assets/sounds/library/pop.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "boing";
    ls.category = "Effects";
    ls.filepath = "assets/sounds/library/boing.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "whoosh";
    ls.category = "Effects";
    ls.filepath = "assets/sounds/library/whoosh.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "splash";
    ls.category = "Effects";
    ls.filepath = "assets/sounds/library/splash.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "laser";
    ls.category = "Effects";
    ls.filepath = "assets/sounds/library/laser.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "drum";
    ls.category = "Music";
    ls.filepath = "assets/sounds/library/drum.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "piano";
    ls.category = "Music";
    ls.filepath = "assets/sounds/library/piano.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "guitar";
    ls.category = "Music";
    ls.filepath = "assets/sounds/library/guitar.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "bell";
    ls.category = "Music";
    ls.filepath = "assets/sounds/library/bell.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "laugh";
    ls.category = "Human";
    ls.filepath = "assets/sounds/library/laugh.wav";
    g_sound_manager.library.push_back(ls);
    
    ls.name = "cheer";
    ls.category = "Human";
    ls.filepath = "assets/sounds/library/cheer.wav";
    g_sound_manager.library.push_back(ls);
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
    g_sound_manager.sounds.clear();
    
    sm_init_library();
    
    SoundItem default_sound;
    default_sound.name = "meow";
    default_sound.filepath = "assets/sounds/meow.wav";
    default_sound.loaded = false;
    default_sound.duration = 0.0f;
    default_sound.volume = 100;
    default_sound.pitch = 0;
    
    if (sound_load(default_sound.name, default_sound.filepath)) {
        default_sound.loaded = true;
        g_sound_manager.sounds.push_back(default_sound);
    }
    
    log_info("SoundManager initialized");
}

void sound_manager_cleanup() {
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].loaded) {
            sound_unload(g_sound_manager.sounds[i].name);
        }
    }
    g_sound_manager.sounds.clear();
    g_sound_manager.library.clear();
    log_info("SoundManager cleaned up");
}

bool sound_manager_add_from_file(const std::string& filepath) {
    if (filepath.empty()) {
        return false;
    }
    
    std::string name = sm_extract_filename(filepath);
    
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].name == name) {
            log_warning("Sound already exists: " + name);
            return false;
        }
    }
    
    if (!sound_load(name, filepath)) {
        log_error("Failed to load sound: " + filepath);
        return false;
    }
    
    SoundItem item;
    item.name = name;
    item.filepath = filepath;
    item.loaded = true;
    item.duration = 0.0f;
    item.volume = 100;
    item.pitch = 0;
    
    g_sound_manager.sounds.push_back(item);
    log_info("Added sound from file: " + name);
    return true;
}

bool sound_manager_add_from_library(const std::string& library_name) {
    LibrarySound* lib_sound = nullptr;
    
    for (size_t i = 0; i < g_sound_manager.library.size(); i++) {
        if (g_sound_manager.library[i].name == library_name) {
            lib_sound = &g_sound_manager.library[i];
            break;
        }
    }
    
    if (!lib_sound) {
        log_warning("Library sound not found: " + library_name);
        return false;
    }
    
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].name == library_name) {
            log_warning("Sound already in project: " + library_name);
            return false;
        }
    }
    
    if (!sound_load(lib_sound->name, lib_sound->filepath)) {
        log_error("Failed to load library sound: " + library_name);
        return false;
    }
    
    SoundItem item;
    item.name = lib_sound->name;
    item.filepath = lib_sound->filepath;
    item.loaded = true;
    item.duration = 0.0f;
    item.volume = 100;
    item.pitch = 0;
    
    g_sound_manager.sounds.push_back(item);
    log_info("Added sound from library: " + library_name);
    return true;
}

bool sound_manager_remove(int index) {
    if (index < 0 || index >= (int)g_sound_manager.sounds.size()) {
        return false;
    }
    
    std::string name = g_sound_manager.sounds[index].name;
    
    if (g_sound_manager.sounds[index].loaded) {
        sound_unload(name);
    }
    
    g_sound_manager.sounds.erase(g_sound_manager.sounds.begin() + index);
    
    if (g_sound_manager.selected_index >= (int)g_sound_manager.sounds.size()) {
        g_sound_manager.selected_index = (int)g_sound_manager.sounds.size() - 1;
    }
    
    log_info("Removed sound: " + name);
    return true;
}

bool sound_manager_remove_by_name(const std::string& name) {
    for (int i = 0; i < (int)g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].name == name) {
            return sound_manager_remove(i);
        }
    }
    return false;
}

SoundItem* sound_manager_get(int index) {
    if (index >= 0 && index < (int)g_sound_manager.sounds.size()) {
        return &g_sound_manager.sounds[index];
    }
    return nullptr;
}

SoundItem* sound_manager_get_by_name(const std::string& name) {
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].name == name) {
            return &g_sound_manager.sounds[i];
        }
    }
    return nullptr;
}

int sound_manager_count() {
    return (int)g_sound_manager.sounds.size();
}

std::vector<std::string> sound_manager_get_names() {
    std::vector<std::string> names;
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        names.push_back(g_sound_manager.sounds[i].name);
    }
    if (names.empty()) {
        names.push_back("meow");
    }
    return names;
}

std::vector<LibrarySound> sound_manager_get_library_by_category(const std::string& category) {
    std::vector<LibrarySound> result;
    for (size_t i = 0; i < g_sound_manager.library.size(); i++) {
        if (category == "All" || g_sound_manager.library[i].category == category) {
            result.push_back(g_sound_manager.library[i]);
        }
    }
    return result;
}

std::vector<std::string> sound_manager_get_categories() {
    std::vector<std::string> categories;
    categories.push_back("All");
    
    for (size_t i = 0; i < g_sound_manager.library.size(); i++) {
        bool found = false;
        for (size_t j = 0; j < categories.size(); j++) {
            if (categories[j] == g_sound_manager.library[i].category) {
                found = true;
                break;
            }
        }
        if (!found) {
            categories.push_back(g_sound_manager.library[i].category);
        }
    }
    return categories;
}

void sound_manager_preview(const std::string& name) {
    play_sound(name, get_sound_volume());
}

void sound_manager_stop_preview() {
    stop_all_sounds();
}

bool sound_manager_save(const std::string& project_path) {
    std::string sounds_file = project_path + "/sounds.txt";
    std::ofstream file(sounds_file.c_str());
    
    if (!file.is_open()) {
        log_error("Cannot save sounds to: " + sounds_file);
        return false;
    }
    
    file << "SOUND_COUNT=" << g_sound_manager.sounds.size() << "\n";
    
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        file << "SOUND=" << g_sound_manager.sounds[i].name << "|";
        file << g_sound_manager.sounds[i].filepath << "|";
        file << g_sound_manager.sounds[i].volume << "|";
        file << g_sound_manager.sounds[i].pitch << "\n";
    }
    
    file.close();
    log_info("Saved sounds: " + std::to_string(g_sound_manager.sounds.size()));
    return true;
}

bool sound_manager_load(const std::string& project_path) {
    std::string sounds_file = project_path + "/sounds.txt";
    std::ifstream file(sounds_file.c_str());
    
    if (!file.is_open()) {
        return false;
    }
    
    for (size_t i = 0; i < g_sound_manager.sounds.size(); i++) {
        if (g_sound_manager.sounds[i].loaded) {
            sound_unload(g_sound_manager.sounds[i].name);
        }
    }
    g_sound_manager.sounds.clear();
    
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line.substr(0, 6) == "SOUND=") {
            std::string data = line.substr(6);
            std::vector<std::string> parts;
            std::stringstream ss(data);
            std::string part;
            while (std::getline(ss, part, '|')) {
                parts.push_back(part);
            }
            
            if (parts.size() >= 2) {
                std::string name = parts[0];
                std::string path = parts[1];
                int vol = 100;
                int pit = 0;
                
                if (parts.size() >= 3) {
                    vol = std::stoi(parts[2]);
                }
                if (parts.size() >= 4) {
                    pit = std::stoi(parts[3]);
                }
                
                if (sound_load(name, path)) {
                    SoundItem item;
                    item.name = name;
                    item.filepath = path;
                    item.loaded = true;
                    item.duration = 0.0f;
                    item.volume = vol;
                    item.pitch = pit;
                    g_sound_manager.sounds.push_back(item);
                }
            }
        }
    }
    
    file.close();
    log_info("Loaded sounds: " + std::to_string(g_sound_manager.sounds.size()));
    return true;
}

void sound_manager_set_visible(bool visible) {
    g_sound_manager.visible = visible;
}

bool sound_manager_is_visible() {
    return g_sound_manager.visible;
}

void sound_manager_open_library_dialog() {
    g_sound_manager.library_dialog_open = true;
    g_sound_manager.file_dialog_open = false;
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
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        sound_manager_add_from_file(std::string(ofn.lpstrFile));
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
    
    roundedBoxRGBA(renderer, px, py, px + pw, py + ph, 8,
                   SM_PANEL_BG.r, SM_PANEL_BG.g, SM_PANEL_BG.b, SM_PANEL_BG.a);
    roundedRectangleRGBA(renderer, px, py, px + pw, py + ph, 8,
                         SM_PANEL_BORDER.r, SM_PANEL_BORDER.g, SM_PANEL_BORDER.b, 255);
    
    roundedBoxRGBA(renderer, px, py, px + pw, py + 35, 8,
                   SM_HEADER_BG.r, SM_HEADER_BG.g, SM_HEADER_BG.b, 255);
    boxRGBA(renderer, px, py + 20, px + pw, py + 35,
            SM_HEADER_BG.r, SM_HEADER_BG.g, SM_HEADER_BG.b, 255);
    
    filledCircleRGBA(renderer, px + 22, py + 17, 10, 255, 255, 255, 200);
    stringRGBA(renderer, px + 40, py + 12, "Sounds", 255, 255, 255, 255);
    
    int content_y = py + 40;
    int content_h = ph - 40 - SOUND_MANAGER_ADD_BTN_HEIGHT - 10;
    
    SDL_Rect clip_rect = {px + 5, content_y, pw - 10, content_h};
    SDL_RenderSetClipRect(renderer, &clip_rect);
    
    int item_y = content_y - g_sound_manager.scroll_offset;
    
    for (int i = 0; i < (int)g_sound_manager.sounds.size(); i++) {
        if (item_y + SOUND_MANAGER_ITEM_HEIGHT > content_y && item_y < content_y + content_h) {
            int item_x = px + 8;
            int item_w = pw - 16;
            int item_h = SOUND_MANAGER_ITEM_HEIGHT;
            
            SDL_Color bg_color;
            if (i == g_sound_manager.selected_index) {
                bg_color = SM_ITEM_SELECTED;
            } else if (i == g_sound_manager.hover_index) {
                bg_color = SM_ITEM_HOVER;
            } else {
                bg_color = SM_ITEM_BG;
            }
            
            roundedBoxRGBA(renderer, item_x, item_y, item_x + item_w, item_y + item_h, 5,
                           bg_color.r, bg_color.g, bg_color.b, bg_color.a);
            
            int play_x = item_x + 8;
            int play_y = item_y + (item_h - SOUND_MANAGER_BUTTON_SIZE) / 2;
            roundedBoxRGBA(renderer, play_x, play_y,
                           play_x + SOUND_MANAGER_BUTTON_SIZE, play_y + SOUND_MANAGER_BUTTON_SIZE, 4,
                           SM_BTN_PLAY.r, SM_BTN_PLAY.g, SM_BTN_PLAY.b, 255);
            
            Sint16 tri_x[3] = {(Sint16)(play_x + 8), (Sint16)(play_x + 8), (Sint16)(play_x + 20)};
            Sint16 tri_y[3] = {(Sint16)(play_y + 6), (Sint16)(play_y + 22), (Sint16)(play_y + 14)};
            filledPolygonRGBA(renderer, tri_x, tri_y, 3, 255, 255, 255, 255);
            
            int del_x = item_x + item_w - SOUND_MANAGER_BUTTON_SIZE - 8;
            int del_y = play_y;
            roundedBoxRGBA(renderer, del_x, del_y,
                           del_x + SOUND_MANAGER_BUTTON_SIZE, del_y + SOUND_MANAGER_BUTTON_SIZE, 4,
                           SM_BTN_DELETE.r, SM_BTN_DELETE.g, SM_BTN_DELETE.b, 255);
            
            thickLineRGBA(renderer, del_x + 8, del_y + 8, del_x + 20, del_y + 20, 2, 255, 255, 255, 255);
            thickLineRGBA(renderer, del_x + 20, del_y + 8, del_x + 8, del_y + 20, 2, 255, 255, 255, 255);
            
            int text_x = play_x + SOUND_MANAGER_BUTTON_SIZE + 12;
            int text_y = item_y + item_h / 2 - 4;
            stringRGBA(renderer, text_x, text_y, g_sound_manager.sounds[i].name.c_str(),
                       SM_TEXT_COLOR.r, SM_TEXT_COLOR.g, SM_TEXT_COLOR.b, 255);
        }
        item_y += SOUND_MANAGER_ITEM_HEIGHT + 5;
    }
    
    SDL_RenderSetClipRect(renderer, NULL);
    
    int add_btn_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - 5;
    int add_btn_x = px + 10;
    int add_btn_w = pw - 20;
    
    roundedBoxRGBA(renderer, add_btn_x, add_btn_y,
                   add_btn_x + add_btn_w, add_btn_y + SOUND_MANAGER_ADD_BTN_HEIGHT, 6,
                   SM_BTN_ADD.r, SM_BTN_ADD.g, SM_BTN_ADD.b, 255);
    
    int plus_cx = add_btn_x + add_btn_w / 2;
    int plus_cy = add_btn_y + SOUND_MANAGER_ADD_BTN_HEIGHT / 2;
    thickLineRGBA(renderer, plus_cx - 10, plus_cy, plus_cx + 10, plus_cy, 3, 255, 255, 255, 255);
    thickLineRGBA(renderer, plus_cx, plus_cy - 10, plus_cx, plus_cy + 10, 3, 255, 255, 255, 255);
    
    stringRGBA(renderer, add_btn_x + 35, add_btn_y + 12, "+ Add Sound", 255, 255, 255, 255);
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
    
    roundedBoxRGBA(renderer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, 8,
                   SM_MENU_BG.r, SM_MENU_BG.g, SM_MENU_BG.b, SM_MENU_BG.a);
    roundedRectangleRGBA(renderer, menu_x, menu_y, menu_x + menu_w, menu_y + menu_h, 8,
                         SM_PANEL_BORDER.r, SM_PANEL_BORDER.g, SM_PANEL_BORDER.b, 255);
    
    int opt1_y = menu_y + 8;
    int opt2_y = menu_y + 44;
    
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    
    if (sm_point_in_rect(mx, my, menu_x + 5, opt1_y, menu_w - 10, 32)) {
        roundedBoxRGBA(renderer, menu_x + 5, opt1_y, menu_x + menu_w - 5, opt1_y + 32, 4,
                       SM_MENU_HOVER.r, SM_MENU_HOVER.g, SM_MENU_HOVER.b, 255);
    }
    stringRGBA(renderer, menu_x + 15, opt1_y + 10, "Choose from Library", 255, 255, 255, 255);
    
    if (sm_point_in_rect(mx, my, menu_x + 5, opt2_y, menu_w - 10, 32)) {
        roundedBoxRGBA(renderer, menu_x + 5, opt2_y, menu_x + menu_w - 5, opt2_y + 32, 4,
                       SM_MENU_HOVER.r, SM_MENU_HOVER.g, SM_MENU_HOVER.b, 255);
    }
    stringRGBA(renderer, menu_x + 15, opt2_y + 10, "Upload Sound", 255, 255, 255, 255);
}

static void sm_render_library_dialog(SDL_Renderer* renderer) {
    int dialog_w = SOUND_LIBRARY_DIALOG_WIDTH;
    int dialog_h = SOUND_LIBRARY_DIALOG_HEIGHT;
    int dialog_x = (WINDOW_WIDTH - dialog_w) / 2;
    int dialog_y = (WINDOW_HEIGHT - dialog_h) / 2;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    boxRGBA(renderer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 150);
    
    roundedBoxRGBA(renderer, dialog_x, dialog_y, dialog_x + dialog_w, dialog_y + dialog_h, 10,
                   SM_DIALOG_BG.r, SM_DIALOG_BG.g, SM_DIALOG_BG.b, SM_DIALOG_BG.a);
    roundedRectangleRGBA(renderer, dialog_x, dialog_y, dialog_x + dialog_w, dialog_y + dialog_h, 10,
                         SM_PANEL_BORDER.r, SM_PANEL_BORDER.g, SM_PANEL_BORDER.b, 255);
    
    roundedBoxRGBA(renderer, dialog_x, dialog_y, dialog_x + dialog_w, dialog_y + 40, 10,
                   SM_DIALOG_HEADER.r, SM_DIALOG_HEADER.g, SM_DIALOG_HEADER.b, 255);
    boxRGBA(renderer, dialog_x, dialog_y + 30, dialog_x + dialog_w, dialog_y + 40,
            SM_DIALOG_HEADER.r, SM_DIALOG_HEADER.g, SM_DIALOG_HEADER.b, 255);
    
    stringRGBA(renderer, dialog_x + 20, dialog_y + 14, "Sound Library", 255, 255, 255, 255);
    
    int close_x = dialog_x + dialog_w - 35;
    int close_y = dialog_y + 8;
    roundedBoxRGBA(renderer, close_x, close_y, close_x + 25, close_y + 25, 4, 180, 60, 60, 255);
    thickLineRGBA(renderer, close_x + 6, close_y + 6, close_x + 19, close_y + 19, 2, 255, 255, 255, 255);
    thickLineRGBA(renderer, close_x + 19, close_y + 6, close_x + 6, close_y + 19, 2, 255, 255, 255, 255);
    
    int cat_x = dialog_x + 10;
    int cat_y = dialog_y + 50;
    int cat_w = 100;
    
    std::vector<std::string> categories = sound_manager_get_categories();
    
    for (size_t i = 0; i < categories.size(); i++) {
        int btn_y = cat_y + (int)i * 35;
        
        SDL_Color cat_bg;
        if (g_sound_manager.selected_category == categories[i]) {
            cat_bg = SM_CATEGORY_SELECTED;
        } else {
            cat_bg = SM_CATEGORY_BG;
        }
        
        roundedBoxRGBA(renderer, cat_x, btn_y, cat_x + cat_w, btn_y + 30, 4,
                       cat_bg.r, cat_bg.g, cat_bg.b, 255);
        stringRGBA(renderer, cat_x + 10, btn_y + 10, categories[i].c_str(), 255, 255, 255, 255);
    }
    
    int list_x = dialog_x + 120;
    int list_y = dialog_y + 50;
    int list_w = dialog_w - 140;
    int list_h = dialog_h - 110;
    
    roundedBoxRGBA(renderer, list_x, list_y, list_x + list_w, list_y + list_h, 6,
                   SM_ITEM_BG.r, SM_ITEM_BG.g, SM_ITEM_BG.b, 200);
    
    SDL_Rect list_clip = {list_x, list_y, list_w, list_h};
    SDL_RenderSetClipRect(renderer, &list_clip);
    
    std::vector<LibrarySound> filtered = sound_manager_get_library_by_category(g_sound_manager.selected_category);
    
    int item_y = list_y + 5 - g_sound_manager.library_scroll_offset;
    
    for (size_t i = 0; i < filtered.size(); i++) {
        if (item_y + 35 > list_y && item_y < list_y + list_h) {
            SDL_Color item_bg;
            if ((int)i == g_sound_manager.library_selected_index) {
                item_bg = SM_ITEM_SELECTED;
            } else {
                item_bg = SM_ITEM_HOVER;
            }
            
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            if (sm_point_in_rect(mx, my, list_x + 5, item_y, list_w - 10, 32)) {
                item_bg = SM_ITEM_HOVER;
            }
            
            if ((int)i == g_sound_manager.library_selected_index) {
                item_bg = SM_ITEM_SELECTED;
            }
            
            roundedBoxRGBA(renderer, list_x + 5, item_y, list_x + list_w - 5, item_y + 32, 4,
                           item_bg.r, item_bg.g, item_bg.b, 255);
            
            int play_btn_x = list_x + 10;
            int play_btn_y = item_y + 4;
            roundedBoxRGBA(renderer, play_btn_x, play_btn_y, play_btn_x + 24, play_btn_y + 24, 3,
                           SM_BTN_PLAY.r, SM_BTN_PLAY.g, SM_BTN_PLAY.b, 255);
            
            Sint16 tri_px[3] = {(Sint16)(play_btn_x + 7), (Sint16)(play_btn_x + 7), (Sint16)(play_btn_x + 17)};
            Sint16 tri_py[3] = {(Sint16)(play_btn_y + 5), (Sint16)(play_btn_y + 19), (Sint16)(play_btn_y + 12)};
            filledPolygonRGBA(renderer, tri_px, tri_py, 3, 255, 255, 255, 255);
            
            stringRGBA(renderer, list_x + 45, item_y + 10, filtered[i].name.c_str(), 255, 255, 255, 255);
        }
        item_y += 38;
    }
    
    SDL_RenderSetClipRect(renderer, NULL);
    
    int btn_y = dialog_y + dialog_h - 50;
    int add_btn_x = dialog_x + dialog_w - 110;
    int cancel_btn_x = dialog_x + dialog_w - 220;
    
    roundedBoxRGBA(renderer, cancel_btn_x, btn_y, cancel_btn_x + 100, btn_y + 35, 5,
                   100, 100, 100, 255);
    stringRGBA(renderer, cancel_btn_x + 30, btn_y + 12, "Cancel", 255, 255, 255, 255);
    
    SDL_Color add_color;
    if (g_sound_manager.library_selected_index >= 0) {
        add_color = SM_BTN_ADD;
    } else {
        add_color = {60, 60, 60, 255};
    }
    
    roundedBoxRGBA(renderer, add_btn_x, btn_y, add_btn_x + 100, btn_y + 35, 5,
                   add_color.r, add_color.g, add_color.b, 255);
    stringRGBA(renderer, add_btn_x + 35, btn_y + 12, "Add", 255, 255, 255, 255);
}

void sound_manager_render() {
    if (!g_sound_manager.visible) {
        return;
    }
    
    SDL_Renderer* renderer = g_sound_manager.renderer;
    
    sm_render_panel(renderer);
    
    if (g_sound_manager.add_menu_open) {
        sm_render_add_menu(renderer);
    }
    
    if (g_sound_manager.library_dialog_open) {
        sm_render_library_dialog(renderer);
    }
}

bool sound_manager_handle_event(SDL_Event* event) {
    if (!g_sound_manager.visible) {
        return false;
    }
    
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
        
        if (sm_point_in_rect(mx, my, px + 8, content_y, pw - 16, content_h)) {
            int item_y = content_y - g_sound_manager.scroll_offset;
            
            for (int i = 0; i < (int)g_sound_manager.sounds.size(); i++) {
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
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        
        if (g_sound_manager.library_dialog_open) {
            int dialog_x = (WINDOW_WIDTH - SOUND_LIBRARY_DIALOG_WIDTH) / 2;
            int dialog_y = (WINDOW_HEIGHT - SOUND_LIBRARY_DIALOG_HEIGHT) / 2;
            int list_x = dialog_x + 120;
            int list_y = dialog_y + 50;
            int list_w = SOUND_LIBRARY_DIALOG_WIDTH - 140;
            int list_h = SOUND_LIBRARY_DIALOG_HEIGHT - 110;
            
            if (sm_point_in_rect(mx, my, list_x, list_y, list_w, list_h)) {
                g_sound_manager.library_scroll_offset -= event->wheel.y * 20;
                
                std::vector<LibrarySound> filtered = sound_manager_get_library_by_category(g_sound_manager.selected_category);
                int max_scroll = (int)filtered.size() * 38 - list_h;
                if (max_scroll < 0) max_scroll = 0;
                
                if (g_sound_manager.library_scroll_offset < 0) {
                    g_sound_manager.library_scroll_offset = 0;
                }
                if (g_sound_manager.library_scroll_offset > max_scroll) {
                    g_sound_manager.library_scroll_offset = max_scroll;
                }
                return true;
            }
        }
        
        if (sm_point_in_rect(mx, my, px, py, pw, ph)) {
            g_sound_manager.scroll_offset -= event->wheel.y * 20;
            
            int max_scroll = (int)g_sound_manager.sounds.size() * (SOUND_MANAGER_ITEM_HEIGHT + 5) - (ph - 60);
            if (max_scroll < 0) max_scroll = 0;
            
            if (g_sound_manager.scroll_offset < 0) {
                g_sound_manager.scroll_offset = 0;
            }
            if (g_sound_manager.scroll_offset > max_scroll) {
                g_sound_manager.scroll_offset = max_scroll;
            }
            return true;
        }
    }
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;
        
        if (g_sound_manager.library_dialog_open) {
            int dialog_w = SOUND_LIBRARY_DIALOG_WIDTH;
            int dialog_h = SOUND_LIBRARY_DIALOG_HEIGHT;
            int dialog_x = (WINDOW_WIDTH - dialog_w) / 2;
            int dialog_y = (WINDOW_HEIGHT - dialog_h) / 2;
            
            int close_x = dialog_x + dialog_w - 35;
            int close_y = dialog_y + 8;
            if (sm_point_in_rect(mx, my, close_x, close_y, 25, 25)) {
                g_sound_manager.library_dialog_open = false;
                return true;
            }
            
            int cat_x = dialog_x + 10;
            int cat_y = dialog_y + 50;
            int cat_w = 100;
            
            std::vector<std::string> categories = sound_manager_get_categories();
            for (size_t i = 0; i < categories.size(); i++) {
                int btn_y = cat_y + (int)i * 35;
                if (sm_point_in_rect(mx, my, cat_x, btn_y, cat_w, 30)) {
                    g_sound_manager.selected_category = categories[i];
                    g_sound_manager.library_selected_index = -1;
                    g_sound_manager.library_scroll_offset = 0;
                    return true;
                }
            }
            
            int list_x = dialog_x + 120;
            int list_y = dialog_y + 50;
            int list_w = dialog_w - 140;
            int list_h = dialog_h - 110;
            
            if (sm_point_in_rect(mx, my, list_x, list_y, list_w, list_h)) {
                std::vector<LibrarySound> filtered = sound_manager_get_library_by_category(g_sound_manager.selected_category);
                int item_y = list_y + 5 - g_sound_manager.library_scroll_offset;
                
                for (size_t i = 0; i < filtered.size(); i++) {
                    if (my >= item_y && my < item_y + 32) {
                        int play_btn_x = list_x + 10;
                        int play_btn_y = item_y + 4;
                        
                        if (sm_point_in_rect(mx, my, play_btn_x, play_btn_y, 24, 24)) {
                            sound_manager_preview(filtered[i].name);
                        } else {
                            g_sound_manager.library_selected_index = (int)i;
                        }
                        return true;
                    }
                    item_y += 38;
                }
            }
            
            int btn_y = dialog_y + dialog_h - 50;
            int add_btn_x = dialog_x + dialog_w - 110;
            int cancel_btn_x = dialog_x + dialog_w - 220;
            
            if (sm_point_in_rect(mx, my, cancel_btn_x, btn_y, 100, 35)) {
                g_sound_manager.library_dialog_open = false;
                return true;
            }
            
            if (sm_point_in_rect(mx, my, add_btn_x, btn_y, 100, 35)) {
                if (g_sound_manager.library_selected_index >= 0) {
                    std::vector<LibrarySound> filtered = sound_manager_get_library_by_category(g_sound_manager.selected_category);
                    if (g_sound_manager.library_selected_index < (int)filtered.size()) {
                        sound_manager_add_from_library(filtered[g_sound_manager.library_selected_index].name);
                        g_sound_manager.library_dialog_open = false;
                    }
                }
                return true;
            }
            
            return true;
        }
        
        if (g_sound_manager.add_menu_open) {
            int menu_w = 180;
            int menu_h = 80;
            int menu_x = px + (pw - menu_w) / 2;
            int menu_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - menu_h - 15;
            
            int opt1_y = menu_y + 8;
            int opt2_y = menu_y + 44;
            
            if (sm_point_in_rect(mx, my, menu_x + 5, opt1_y, menu_w - 10, 32)) {
                g_sound_manager.add_menu_open = false;
                sound_manager_open_library_dialog();
                return true;
            }
            
            if (sm_point_in_rect(mx, my, menu_x + 5, opt2_y, menu_w - 10, 32)) {
                g_sound_manager.add_menu_open = false;
                sound_manager_open_file_dialog();
                return true;
            }
            
            if (!sm_point_in_rect(mx, my, menu_x, menu_y, menu_w, menu_h)) {
                g_sound_manager.add_menu_open = false;
                return true;
            }
            
            return true;
        }
        
        int add_btn_y = py + ph - SOUND_MANAGER_ADD_BTN_HEIGHT - 5;
        int add_btn_x = px + 10;
        int add_btn_w = pw - 20;
        
        if (sm_point_in_rect(mx, my, add_btn_x, add_btn_y, add_btn_w, SOUND_MANAGER_ADD_BTN_HEIGHT)) {
            g_sound_manager.add_menu_open = !g_sound_manager.add_menu_open;
            return true;
        }
        
        int content_y = py + 40;
        int content_h = ph - 40 - SOUND_MANAGER_ADD_BTN_HEIGHT - 10;
        
        if (sm_point_in_rect(mx, my, px + 8, content_y, pw - 16, content_h)) {
            int item_y = content_y - g_sound_manager.scroll_offset;
            
            for (int i = 0; i < (int)g_sound_manager.sounds.size(); i++) {
                if (my >= item_y && my < item_y + SOUND_MANAGER_ITEM_HEIGHT) {
                    int item_x = px + 8;
                    int item_w = pw - 16;
                    int item_h = SOUND_MANAGER_ITEM_HEIGHT;
                    
                    int play_x = item_x + 8;
                    int play_y = item_y + (item_h - SOUND_MANAGER_BUTTON_SIZE) / 2;
                    
                    if (sm_point_in_rect(mx, my, play_x, play_y, SOUND_MANAGER_BUTTON_SIZE, SOUND_MANAGER_BUTTON_SIZE)) {
                        sound_manager_preview(g_sound_manager.sounds[i].name);
                        return true;
                    }
                    
                    int del_x = item_x + item_w - SOUND_MANAGER_BUTTON_SIZE - 8;
                    int del_y = play_y;
                    
                    if (sm_point_in_rect(mx, my, del_x, del_y, SOUND_MANAGER_BUTTON_SIZE, SOUND_MANAGER_BUTTON_SIZE)) {
                        sound_manager_remove(i);
                        return true;
                    }
                    
                    g_sound_manager.selected_index = i;
                    return true;
                }
                item_y += SOUND_MANAGER_ITEM_HEIGHT + 5;
            }
        }
        
        if (sm_point_in_rect(mx, my, px, py, pw, ph)) {
            return true;
        }
    }
    
    return false;
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
