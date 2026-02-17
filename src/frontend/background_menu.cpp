#include "background_menu.h"
#include "../utils/logger.h"
#include "../common/globals.h"
#include "draw.h"
#include <cstdio>
#include <cstring>
#include <cmath>

static SDL_Texture* create_color_texture(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, int w, int h) {
    SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!tex) return nullptr;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, tex);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    return tex;
}

static SDL_Texture* create_thumbnail(SDL_Renderer* renderer, SDL_Texture* source, int size) {
    SDL_Texture* thumb = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size);
    if (!thumb) return nullptr;
    SDL_SetTextureBlendMode(thumb, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, thumb);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_Rect dst = {0, 0, size, size};
    SDL_RenderCopy(renderer, source, nullptr, &dst);
    SDL_SetRenderTarget(renderer, nullptr);
    return thumb;
}

void bg_menu_init(BackgroundMenu* menu, int panel_x, int panel_y) {
    menu->is_open = false;
    menu->selected_index = -1;
    menu->current_bg_index = -1;
    menu->item_count = 0;
    menu->show_color_picker = false;
    menu->custom_r = 255;
    menu->custom_g = 255;
    menu->custom_b = 255;
    menu->panel_x = panel_x;
    menu->panel_y = panel_y;
    for (int i = 0; i < MAX_BACKGROUNDS; i++) {
        menu->items[i].texture = nullptr;
        menu->items[i].thumbnail = nullptr;
        menu->items[i].is_solid_color = false;
    }
    log_info("Background menu initialized");
}

void bg_menu_destroy(BackgroundMenu* menu) {
    for (int i = 0; i < menu->item_count; i++) {
        if (menu->items[i].texture) SDL_DestroyTexture(menu->items[i].texture);
        if (menu->items[i].thumbnail) SDL_DestroyTexture(menu->items[i].thumbnail);
        menu->items[i].texture = nullptr;
        menu->items[i].thumbnail = nullptr;
    }
    menu->item_count = 0;
}

int bg_menu_add_image(BackgroundMenu* menu, const char* name, const char* filepath, SDL_Renderer* renderer) {
    if (menu->item_count >= MAX_BACKGROUNDS) {
        log_warning("BG: max backgrounds reached");
        return -1;
    }
    SDL_Surface* surface = IMG_Load(filepath);
    if (!surface) {
        log_warning("BG: failed to load " + std::string(filepath));
        return -1;
    }
    int idx = menu->item_count;
    menu->items[idx].name = name;
    menu->items[idx].filepath = filepath;
    menu->items[idx].texture = SDL_CreateTextureFromSurface(renderer, surface);
    menu->items[idx].width = surface->w;
    menu->items[idx].height = surface->h;
    menu->items[idx].is_solid_color = false;
    menu->items[idx].thumbnail = create_thumbnail(renderer, menu->items[idx].texture, BG_THUMB_SIZE);
    SDL_FreeSurface(surface);
    menu->item_count++;
    log_info("BG: added image background: " + std::string(name));
    return idx;
}

int bg_menu_add_solid_color(BackgroundMenu* menu, const char* name, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer* renderer) {
    if (menu->item_count >= MAX_BACKGROUNDS) {
        log_warning("BG: max backgrounds reached");
        return -1;
    }
    int idx = menu->item_count;
    menu->items[idx].name = name;
    menu->items[idx].filepath = "";
    menu->items[idx].is_solid_color = true;
    menu->items[idx].solid_r = r;
    menu->items[idx].solid_g = g;
    menu->items[idx].solid_b = b;
    menu->items[idx].texture = create_color_texture(renderer, r, g, b, 480, 360);
    menu->items[idx].width = 480;
    menu->items[idx].height = 360;
    menu->items[idx].thumbnail = create_thumbnail(renderer, menu->items[idx].texture, BG_THUMB_SIZE);
    menu->item_count++;
    log_info("BG: added solid color background: " + std::string(name));
    return idx;
}

void bg_menu_remove_item(BackgroundMenu* menu, int index) {
    if (index < 0 || index >= menu->item_count) return;
    if (menu->items[index].texture) SDL_DestroyTexture(menu->items[index].texture);
    if (menu->items[index].thumbnail) SDL_DestroyTexture(menu->items[index].thumbnail);
    for (int i = index; i < menu->item_count - 1; i++) {
        menu->items[i] = menu->items[i + 1];
    }
    menu->item_count--;
    menu->items[menu->item_count].texture = nullptr;
    menu->items[menu->item_count].thumbnail = nullptr;
    if (menu->selected_index == index) {
        menu->selected_index = (menu->item_count > 0) ? 0 : -1;
    } else if (menu->selected_index > index) {
        menu->selected_index--;
    }
    if (menu->current_bg_index == index) {
        menu->current_bg_index = (menu->item_count > 0) ? 0 : -1;
    } else if (menu->current_bg_index > index) {
        menu->current_bg_index--;
    }
    log_info("BG: removed background at index " + std::to_string(index));
}

void bg_menu_select(BackgroundMenu* menu, int index) {
    if (index >= 0 && index < menu->item_count) {
        menu->selected_index = index;
        menu->current_bg_index = index;
        log_info("BG: selected background " + std::to_string(index));
    }
}

void bg_menu_toggle(BackgroundMenu* menu) {
    menu->is_open = !menu->is_open;
}

void bg_render_stage_background(BackgroundMenu* menu, SDL_Renderer* renderer, int stage_x, int stage_y, int stage_w, int stage_h) {
    SDL_Rect stage_rect = {stage_x, stage_y, stage_w, stage_h};
    if (menu->current_bg_index >= 0 && menu->current_bg_index < menu->item_count && menu->items[menu->current_bg_index].texture) {
        SDL_RenderCopy(renderer, menu->items[menu->current_bg_index].texture, nullptr, &stage_rect);
    } else {
        SDL_SetRenderDrawColor(renderer, menu->custom_r, menu->custom_g, menu->custom_b, 255);
        SDL_RenderFillRect(renderer, &stage_rect);
    }
}

void bg_menu_render(BackgroundMenu* menu, SDL_Renderer* renderer) {
    if (!menu->is_open) return;

    SDL_Rect panel = {menu->panel_x, menu->panel_y, BG_PANEL_W, BG_PANEL_H};
    SDL_SetRenderDrawColor(renderer, 245, 245, 248, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &panel);

    draw_text(renderer, menu->panel_x + 10, menu->panel_y + 8, "Backgrounds", COLOR_GRAY);

    SDL_Rect close_btn = {menu->panel_x + BG_PANEL_W - 22, menu->panel_y + 5, 16, 16};
    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);
    SDL_RenderFillRect(renderer, &close_btn);
    draw_text(renderer, close_btn.x + 3, close_btn.y + 2, "X", COLOR_BLACK);

    int cols = 3;
    int padding = 6;
    int start_x = menu->panel_x + 10;
    int start_y = menu->panel_y + 28;

    for (int i = 0; i < menu->item_count; i++) {
        int col = i % cols;
        int row = i / cols;
        int tx = start_x + col * (BG_THUMB_SIZE + padding);
        int ty = start_y + row * (BG_THUMB_SIZE + padding + 14);
        SDL_Rect thumb_rect = {tx, ty, BG_THUMB_SIZE, BG_THUMB_SIZE};

        if (menu->items[i].thumbnail) {
            SDL_RenderCopy(renderer, menu->items[i].thumbnail, nullptr, &thumb_rect);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderFillRect(renderer, &thumb_rect);
        }

        if (i == menu->current_bg_index) {
            SDL_SetRenderDrawColor(renderer, 0, 120, 255, 255);
            SDL_RenderDrawRect(renderer, &thumb_rect);
            SDL_Rect outer = {tx - 1, ty - 1, BG_THUMB_SIZE + 2, BG_THUMB_SIZE + 2};
            SDL_RenderDrawRect(renderer, &outer);
        } else {
            SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
            SDL_RenderDrawRect(renderer, &thumb_rect);
        }

        draw_text(renderer, tx + 2, ty + BG_THUMB_SIZE + 2, menu->items[i].name.c_str(), COLOR_GRAY);

        SDL_Rect del_btn = {tx + BG_THUMB_SIZE - 10, ty + 2, 9, 9};
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 200);
        SDL_RenderFillRect(renderer, &del_btn);
        draw_text(renderer, del_btn.x + 1, del_btn.y, "x", COLOR_WHITE);
    }

    int row_after = menu->item_count / cols;
    int col_after = menu->item_count % cols;
    int btn_y = start_y + row_after * (BG_THUMB_SIZE + padding + 14);
    if (col_after == 0 && menu->item_count > 0) {
        btn_y = start_y + row_after * (BG_THUMB_SIZE + padding + 14);
    }

    SDL_Rect add_img_btn = {menu->panel_x + 10, btn_y + BG_THUMB_SIZE + 20, 85, 22};
    SDL_SetRenderDrawColor(renderer, 66, 133, 244, 255);
    SDL_RenderFillRect(renderer, &add_img_btn);
    draw_text(renderer, add_img_btn.x + 5, add_img_btn.y + 5, "+ Image", COLOR_WHITE);

    SDL_Rect add_white_btn = {menu->panel_x + 105, btn_y + BG_THUMB_SIZE + 20, 85, 22};
    SDL_SetRenderDrawColor(renderer, 76, 175, 80, 255);
    SDL_RenderFillRect(renderer, &add_white_btn);
    draw_text(renderer, add_white_btn.x + 5, add_white_btn.y + 5, "+ Color", COLOR_WHITE);

    SDL_Rect color_preview = {menu->panel_x + BG_PANEL_W - 30, menu->panel_y + BG_PANEL_H - 28, 20, 20};
    SDL_SetRenderDrawColor(renderer, menu->custom_r, menu->custom_g, menu->custom_b, 255);
    SDL_RenderFillRect(renderer, &color_preview);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &color_preview);
}

int bg_menu_handle_click(BackgroundMenu* menu, int mx, int my, SDL_Renderer* renderer) {
    if (!menu->is_open) return 0;

    if (mx < menu->panel_x || mx > menu->panel_x + BG_PANEL_W || my < menu->panel_y || my > menu->panel_y + BG_PANEL_H) {
        return 0;
    }

    SDL_Rect close_btn = {menu->panel_x + BG_PANEL_W - 22, menu->panel_y + 5, 16, 16};
    if (mx >= close_btn.x && mx <= close_btn.x + close_btn.w && my >= close_btn.y && my <= close_btn.y + close_btn.h) {
        menu->is_open = false;
        return 1;
    }

    int cols = 3;
    int padding = 6;
    int start_x = menu->panel_x + 10;
    int start_y = menu->panel_y + 28;

    for (int i = 0; i < menu->item_count; i++) {
        int col = i % cols;
        int row = i / cols;
        int tx = start_x + col * (BG_THUMB_SIZE + padding);
        int ty = start_y + row * (BG_THUMB_SIZE + padding + 14);

        SDL_Rect del_btn = {tx + BG_THUMB_SIZE - 10, ty + 2, 9, 9};
        if (mx >= del_btn.x && mx <= del_btn.x + del_btn.w && my >= del_btn.y && my <= del_btn.y + del_btn.h) {
            bg_menu_remove_item(menu, i);
            return 1;
        }

        if (mx >= tx && mx <= tx + BG_THUMB_SIZE && my >= ty && my <= ty + BG_THUMB_SIZE) {
            bg_menu_select(menu, i);
            return 1;
        }
    }

    int row_after = menu->item_count / cols;
    int btn_y = start_y + row_after * (BG_THUMB_SIZE + padding + 14);

    SDL_Rect add_img_btn = {menu->panel_x + 10, btn_y + BG_THUMB_SIZE + 20, 85, 22};
    if (mx >= add_img_btn.x && mx <= add_img_btn.x + add_img_btn.w && my >= add_img_btn.y && my <= add_img_btn.y + add_img_btn.h) {
        char default_name[32];
        snprintf(default_name, sizeof(default_name), "BG_%d", menu->item_count + 1);
        bg_menu_add_image(menu, default_name, "../assets/default_bg.png", renderer);
        return 1;
    }

    SDL_Rect add_color_btn = {menu->panel_x + 105, btn_y + BG_THUMB_SIZE + 20, 85, 22};
    if (mx >= add_color_btn.x && mx <= add_color_btn.x + add_color_btn.w && my >= add_color_btn.y && my <= add_color_btn.y + add_color_btn.h) {
        Uint8 colors_r[] = {255, 135, 100, 255, 0, 255, 200};
        Uint8 colors_g[] = {255, 206, 149, 165, 191, 127, 200};
        Uint8 colors_b[] = {255, 235, 237, 0, 255, 80, 200};
        int ci = menu->item_count % 7;
        char cname[32];
        snprintf(cname, sizeof(cname), "Color_%d", menu->item_count + 1);
        bg_menu_add_solid_color(menu, cname, colors_r[ci], colors_g[ci], colors_b[ci], renderer);
        return 1;
    }

    return 1;
}
