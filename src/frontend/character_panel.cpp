#include "character_panel.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include "draw.h"
#include <cstdio>

void cpanel_init(CharacterPanel* panel, int px, int py, int pw, int ph) {
    panel->count = 0;
    panel->selected_index = -1;
    panel->show_add_menu = false;
    panel->panel_x = px;
    panel->panel_y = py;
    panel->panel_w = pw;
    panel->panel_h = ph;
    for (int i = 0; i < CPANEL_MAX_CHARS; i++) {
        panel->items[i].thumbnail = nullptr;
        panel->items[i].is_visible = true;
        panel->items[i].is_selected = false;
    }
    log_info("Character panel initialized");
}

int cpanel_add(CharacterPanel* panel, const char* name, SDL_Texture* thumb, float x, float y) {
    if (panel->count >= CPANEL_MAX_CHARS) {
        log_warning("CPANEL: max characters reached");
        return -1;
    }
    int idx = panel->count;
    panel->items[idx].name = name;
    panel->items[idx].thumbnail = thumb;
    panel->items[idx].is_visible = true;
    panel->items[idx].is_selected = false;
    panel->items[idx].pos_x = x;
    panel->items[idx].pos_y = y;
    panel->items[idx].angle = 0.0f;
    panel->count++;

    if (panel->count == 1) {
        cpanel_select(panel, 0);
    }
    log_info("CPANEL: added character: " + std::string(name));
    return idx;
}

void cpanel_remove(CharacterPanel* panel, int index) {
    if (index < 0 || index >= panel->count) return;
    if (panel->count <= 1) {
        log_warning("CPANEL: cannot remove last character");
        return;
    }

    for (int i = index; i < panel->count - 1; i++) {
        panel->items[i] = panel->items[i + 1];
    }
    panel->items[panel->count - 1].thumbnail = nullptr;
    panel->count--;

    if (panel->selected_index == index) {
        panel->selected_index = 0;
        panel->items[0].is_selected = true;
    } else if (panel->selected_index > index) {
        panel->selected_index--;
    }

    log_info("CPANEL: removed character at index " + std::to_string(index));
}

void cpanel_select(CharacterPanel* panel, int index) {
    if (index < 0 || index >= panel->count) return;
    for (int i = 0; i < panel->count; i++) {
        panel->items[i].is_selected = false;
    }
    panel->items[index].is_selected = true;
    panel->selected_index = index;
    log_info("CPANEL: selected character " + std::to_string(index));
}

void cpanel_toggle_visibility(CharacterPanel* panel, int index) {
    if (index < 0 || index >= panel->count) return;
    panel->items[index].is_visible = !panel->items[index].is_visible;
    log_info("CPANEL: toggled visibility for character " + std::to_string(index));
}

int cpanel_get_selected(CharacterPanel* panel) {
    return panel->selected_index;
}

void cpanel_render(CharacterPanel* panel, SDL_Renderer* renderer) {
    SDL_Rect bg = {panel->panel_x, panel->panel_y, panel->panel_w, panel->panel_h};
    SDL_SetRenderDrawColor(renderer, 238, 238, 242, 255);
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &bg);

    draw_text(renderer, panel->panel_x + 6, panel->panel_y + 4, "Sprites:", COLOR_GRAY);

    int start_x = panel->panel_x + 8;
    int thumb_y = panel->panel_y + 20;
    int spacing = CPANEL_THUMB_SIZE + 8;

    for (int i = 0; i < panel->count; i++) {
        int tx = start_x + i * spacing;

        if (tx + CPANEL_THUMB_SIZE > panel->panel_x + panel->panel_w - 10) break;

        SDL_Rect thumb_rect = {tx, thumb_y, CPANEL_THUMB_SIZE, CPANEL_THUMB_SIZE};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &thumb_rect);

        if (panel->items[i].thumbnail) {
            SDL_RenderCopy(renderer, panel->items[i].thumbnail, nullptr, &thumb_rect);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 220, 255);
            SDL_RenderFillRect(renderer, &thumb_rect);
            draw_text(renderer, tx + 15, thumb_y + 20, "?", COLOR_GRAY);
        }

        if (panel->items[i].is_selected) {
            SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
            for (int b = 0; b < 3; b++) {
                SDL_Rect border = {tx - b, thumb_y - b, CPANEL_THUMB_SIZE + 2 * b, CPANEL_THUMB_SIZE + 2 * b};
                SDL_RenderDrawRect(renderer, &border);
            }
        } else {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            SDL_RenderDrawRect(renderer, &thumb_rect);
        }

        if (!panel->items[i].is_visible) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 140);
            SDL_RenderFillRect(renderer, &thumb_rect);
            aalineRGBA(renderer, tx + 4, thumb_y + 4, tx + CPANEL_THUMB_SIZE - 4, thumb_y + CPANEL_THUMB_SIZE - 4, 200, 50, 50, 220);
            aalineRGBA(renderer, tx + CPANEL_THUMB_SIZE - 4, thumb_y + 4, tx + 4, thumb_y + CPANEL_THUMB_SIZE - 4, 200, 50, 50, 220);
        }

        if (panel->count > 1) {
            SDL_Rect del_btn = {tx + CPANEL_THUMB_SIZE - 11, thumb_y + 2, 10, 10};
            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 210);
            SDL_RenderFillRect(renderer, &del_btn);
            draw_text(renderer, del_btn.x + 2, del_btn.y + 0, "x", COLOR_WHITE);
        }

        SDL_Rect vis_btn = {tx + 1, thumb_y + 1, 12, 10};
        if (panel->items[i].is_visible) {
            SDL_SetRenderDrawColor(renderer, 50, 180, 50, 200);
        } else {
            SDL_SetRenderDrawColor(renderer, 180, 50, 50, 200);
        }
        SDL_RenderFillRect(renderer, &vis_btn);
        draw_text(renderer, vis_btn.x + 1, vis_btn.y + 0, panel->items[i].is_visible ? "V" : "H", COLOR_WHITE);

        draw_text(renderer, tx + 2, thumb_y + CPANEL_THUMB_SIZE + 3, panel->items[i].name.c_str(), COLOR_GRAY);
    }

    int add_x = start_x + panel->count * spacing;
    if (add_x + CPANEL_THUMB_SIZE <= panel->panel_x + panel->panel_w - 10) {
        SDL_Rect add_btn = {add_x, thumb_y, CPANEL_THUMB_SIZE, CPANEL_THUMB_SIZE};
        SDL_SetRenderDrawColor(renderer, 210, 240, 210, 255);
        SDL_RenderFillRect(renderer, &add_btn);
        SDL_SetRenderDrawColor(renderer, 100, 190, 100, 255);
        SDL_RenderDrawRect(renderer, &add_btn);
        draw_text(renderer, add_x + CPANEL_THUMB_SIZE / 2 - 4, thumb_y + CPANEL_THUMB_SIZE / 2 - 6, "+", COLOR_GREEN);
    }
}

int cpanel_handle_click(CharacterPanel* panel, int mx, int my) {
    if (mx < panel->panel_x || mx > panel->panel_x + panel->panel_w || my < panel->panel_y || my > panel->panel_y + panel->panel_h) {
        return -1;
    }

    int start_x = panel->panel_x + 8;
    int thumb_y = panel->panel_y + 20;
    int spacing = CPANEL_THUMB_SIZE + 8;

    for (int i = 0; i < panel->count; i++) {
        int tx = start_x + i * spacing;

        if (panel->count > 1) {
            SDL_Rect del_btn = {tx + CPANEL_THUMB_SIZE - 11, thumb_y + 2, 10, 10};
            if (mx >= del_btn.x && mx <= del_btn.x + del_btn.w && my >= del_btn.y && my <= del_btn.y + del_btn.h) {
                cpanel_remove(panel, i);
                return 100;
            }
        }

        SDL_Rect vis_btn = {tx + 1, thumb_y + 1, 12, 10};
        if (mx >= vis_btn.x && mx <= vis_btn.x + vis_btn.w && my >= vis_btn.y && my <= vis_btn.y + vis_btn.h) {
            cpanel_toggle_visibility(panel, i);
            return 200 + i;
        }

        if (mx >= tx && mx <= tx + CPANEL_THUMB_SIZE && my >= thumb_y && my <= thumb_y + CPANEL_THUMB_SIZE) {
            cpanel_select(panel, i);
            return i;
        }
    }

    int add_x = start_x + panel->count * spacing;
    if (mx >= add_x && mx <= add_x + CPANEL_THUMB_SIZE && my >= thumb_y && my <= thumb_y + CPANEL_THUMB_SIZE) {
        panel->show_add_menu = true;
        return 300;
    }

    return -1;
}
