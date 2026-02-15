#include "menu.h"
#include "../common/definitions.h"
#include <cstring>

static Menu g_menus[2];  
static int  g_menu_count = 2;


static const unsigned char FONT_5x7[][7] = {
    // A
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    // B
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    // C
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    // D
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    // E
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    // F
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    // G
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    // H
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    // I
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    // J
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    // K
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    // L
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    // M
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    // N
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    // O
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    // P
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    // Q
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    // R
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    // S
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    // T
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    // U
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    // V
    {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    // W
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},
    // X
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    // Y
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    // Z
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

static const unsigned char FONT_LOWER_5x7[][7] = {
    // a
    {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F},
    // b
    {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E},
    // c
    {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E},
    // d
    {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F},
    // e
    {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E},
    // f
    {0x06,0x09,0x08,0x1C,0x08,0x08,0x08},
    // g
    {0x00,0x00,0x0F,0x11,0x0F,0x01,0x0E},
    // h
    {0x10,0x10,0x16,0x19,0x11,0x11,0x11},
    // i
    {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E},
    // j
    {0x02,0x00,0x06,0x02,0x02,0x12,0x0C},
    // k
    {0x10,0x10,0x12,0x14,0x18,0x14,0x12},
    // l
    {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E},
    // m
    {0x00,0x00,0x1A,0x15,0x15,0x11,0x11},
    // n
    {0x00,0x00,0x16,0x19,0x11,0x11,0x11},
    // o
    {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E},
    // p
    {0x00,0x00,0x1E,0x11,0x1E,0x10,0x10},
    // q
    {0x00,0x00,0x0F,0x11,0x0F,0x01,0x01},
    // r
    {0x00,0x00,0x16,0x19,0x10,0x10,0x10},
    // s
    {0x00,0x00,0x0E,0x10,0x0E,0x01,0x1E},
    // t
    {0x08,0x08,0x1C,0x08,0x08,0x09,0x06},
    // u
    {0x00,0x00,0x11,0x11,0x11,0x13,0x0D},
    // v
    {0x00,0x00,0x11,0x11,0x11,0x0A,0x04},
    // w
    {0x00,0x00,0x11,0x11,0x15,0x15,0x0A},
    // x
    {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11},
    // y
    {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E},
    // z
    {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F},
};

static void draw_char(SDL_Renderer* r, char ch, int cx, int cy, int scale,
                       Uint8 cr, Uint8 cg, Uint8 cb)
{
    const unsigned char* glyph = nullptr;

    if (ch >= 'A' && ch <= 'Z') {
        glyph = FONT_5x7[ch - 'A'];
    } else if (ch >= 'a' && ch <= 'z') {
        glyph = FONT_LOWER_5x7[ch - 'a'];
    } else if (ch == ' ') {
        return;
    } else {
        return;
    }

    SDL_SetRenderDrawColor(r, cr, cg, cb, 255);

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row] & (0x10 >> col)) {
                SDL_Rect px = { cx + col * scale, cy + row * scale, scale, scale };
                SDL_RenderFillRect(r, &px);
            }
        }
    }
}

static void draw_text(SDL_Renderer* r, const std::string& text, int x, int y, int scale,
                       Uint8 cr, Uint8 cg, Uint8 cb)
{
    int cursor_x = x;
    for (size_t i = 0; i < text.size(); i++) {
        draw_char(r, text[i], cursor_x, y, scale, cr, cg, cb);
        cursor_x += 6 * scale;
    }
}
void menu_init() {
    g_menus[0].title  = "File";
    g_menus[0].x      = 4;
    g_menus[0].y      = 0;
    g_menus[0].width  = 40;
    g_menus[0].height = MENU_BAR_OFFSET;
    g_menus[0].is_open = false;
    g_menus[0].title_highlighted = false;
    g_menus[0].item_width  = 120;
    g_menus[0].item_height = 24;
    g_menus[0].items.clear();
    g_menus[0].items.push_back(MenuItem("New"));
    g_menus[0].items.push_back(MenuItem("Save"));
    g_menus[0].items.push_back(MenuItem("Load"));
    g_menus[0].items.push_back(MenuItem("Exit"));

    g_menus[1].title  = "Help";
    g_menus[1].x      = 52;
    g_menus[1].y      = 0;
    g_menus[1].width  = 40;
    g_menus[1].height = MENU_BAR_OFFSET;
    g_menus[1].is_open = false;
    g_menus[1].title_highlighted = false;
    g_menus[1].item_width  = 140;
    g_menus[1].item_height = 24;
    g_menus[1].items.clear();
    g_menus[1].items.push_back(MenuItem("System Logger"));
    g_menus[1].items.push_back(MenuItem("Debug Info"));
    g_menus[1].items.push_back(MenuItem("About"));
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

        int text_y = (MENU_BAR_OFFSET - 7 * 2) / 2;
        draw_text(renderer, m.title, m.x + 6, text_y, 2, 210, 210, 215);

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

                draw_text(renderer, m.items[k].label,
                          dropdown_x + 10, item_y + 5, 2,
                          200, 200, 205);
            }
        }
    }
}
