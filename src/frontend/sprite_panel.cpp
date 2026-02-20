#include "sprite_panel.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <cmath>
#include <cstdio>
#include <vector>

static TTF_Font* s_panelFont      = nullptr;
static TTF_Font* s_panelFontBold  = nullptr;
static TTF_Font* s_panelFontSmall = nullptr;
static bool      s_fontsAttempted = false;

struct PanelColor {
    Uint8 r, g, b, a;
};

static const PanelColor COL_BG_DARK       = {30,  30,  30,  255};
static const PanelColor COL_BG_SECTION    = {37,  37,  38,  255};
static const PanelColor COL_BG_ROW_A      = {42,  42,  45,  255};
static const PanelColor COL_BG_ROW_B      = {37,  37,  40,  255};
static const PanelColor COL_HEADER_BG     = {45,  45,  48,  255};
static const PanelColor COL_BORDER        = {58,  58,  62,  255};
static const PanelColor COL_BORDER_LIGHT  = {70,  70,  75,  255};
static const PanelColor COL_TEXT_PRIMARY   = {220, 220, 220, 255};
static const PanelColor COL_TEXT_SECONDARY = {150, 150, 155, 255};
static const PanelColor COL_TEXT_VALUE     = {100, 200, 255, 255};
static const PanelColor COL_TEXT_LABEL     = {180, 180, 185, 255};
static const PanelColor COL_ACCENT_BLUE    = {50,  120, 220, 255};
static const PanelColor COL_ACCENT_GREEN   = {80,  200, 120, 255};
static const PanelColor COL_ACCENT_ORANGE  = {220, 160, 50,  255};
static const PanelColor COL_ACCENT_PURPLE  = {160, 100, 220, 255};
static const PanelColor COL_ACCENT_RED     = {220, 80,  80,  255};
static const PanelColor COL_ACCENT_TEAL    = {70,  200, 190, 255};
static const PanelColor COL_COORD_BG       = {28,  28,  32,  255};
static const PanelColor COL_SEPARATOR      = {55,  55,  60,  255};
static const PanelColor COL_BADGE_BG       = {50,  50,  55,  255};

static void set_color(SDL_Renderer* r, PanelColor c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

static void fill_rect(SDL_Renderer* r, int x, int y, int w, int h, PanelColor c) {
    set_color(r, c);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderFillRect(r, &rc);
}

static void draw_horizontal_line(SDL_Renderer* r, int x1, int x2, int y, PanelColor c) {
    set_color(r, c);
    SDL_RenderDrawLine(r, x1, y, x2, y);
}

static void fill_rounded_rect(SDL_Renderer* r, int x, int y, int w, int h, int rad, PanelColor c) {
    if (rad < 1 || w < 2 * rad || h < 2 * rad) {
        fill_rect(r, x, y, w, h, c);
        return;
    }
    set_color(r, c);

    SDL_Rect center = {x + rad, y, w - 2 * rad, h};
    SDL_RenderFillRect(r, &center);
    SDL_Rect left = {x, y + rad, rad, h - 2 * rad};
    SDL_RenderFillRect(r, &left);
    SDL_Rect right = {x + w - rad, y + rad, rad, h - 2 * rad};
    SDL_RenderFillRect(r, &right);

    for (int dy = 0; dy <= rad; dy++) {
        int dx = (int)std::sqrt((float)(rad * rad - dy * dy));
        SDL_RenderDrawLine(r, x + rad - dx, y + rad - dy, x + rad, y + rad - dy);
        SDL_RenderDrawLine(r, x + w - rad, y + rad - dy, x + w - rad + dx, y + rad - dy);
        SDL_RenderDrawLine(r, x + rad - dx, y + h - rad + dy, x + rad, y + h - rad + dy);
        SDL_RenderDrawLine(r, x + w - rad, y + h - rad + dy, x + w - rad + dx, y + h - rad + dy);
    }
}

static void draw_rounded_rect_outline(SDL_Renderer* r, int x, int y, int w, int h, int rad, PanelColor c) {
    if (rad < 1) {
        set_color(r, c);
        SDL_Rect rc = {x, y, w, h};
        SDL_RenderDrawRect(r, &rc);
        return;
    }
    set_color(r, c);

    SDL_RenderDrawLine(r, x + rad, y, x + w - rad - 1, y);
    SDL_RenderDrawLine(r, x + rad, y + h - 1, x + w - rad - 1, y + h - 1);
    SDL_RenderDrawLine(r, x, y + rad, x, y + h - rad - 1);
    SDL_RenderDrawLine(r, x + w - 1, y + rad, x + w - 1, y + h - rad - 1);

    int segments = rad * 4;
    if (segments < 16) segments = 16;
    float step = (3.14159265f / 2.0f) / (float)segments;

    for (int i = 0; i < segments; i++) {
        float a1 = i * step;
        float a2 = (i + 1) * step;
        int cx, cy;

        cx = x + rad; cy = y + rad;
        SDL_RenderDrawLine(r, cx - (int)(rad * std::cos(a1)), cy - (int)(rad * std::sin(a1)),
                              cx - (int)(rad * std::cos(a2)), cy - (int)(rad * std::sin(a2)));

        cx = x + w - 1 - rad; cy = y + rad;
        SDL_RenderDrawLine(r, cx + (int)(rad * std::cos(a1)), cy - (int)(rad * std::sin(a1)),
                              cx + (int)(rad * std::cos(a2)), cy - (int)(rad * std::sin(a2)));

        cx = x + rad; cy = y + h - 1 - rad;
        SDL_RenderDrawLine(r, cx - (int)(rad * std::cos(a1)), cy + (int)(rad * std::sin(a1)),
                              cx - (int)(rad * std::cos(a2)), cy + (int)(rad * std::sin(a2)));

        cx = x + w - 1 - rad; cy = y + h - 1 - rad;
        SDL_RenderDrawLine(r, cx + (int)(rad * std::cos(a1)), cy + (int)(rad * std::sin(a1)),
                              cx + (int)(rad * std::cos(a2)), cy + (int)(rad * std::sin(a2)));
    }
}

static void draw_gradient_v_rounded(SDL_Renderer* r, int x, int y, int w, int h, int rad,
                                      PanelColor top, PanelColor bot) {
    for (int i = 0; i < h; i++) {
        float t = (float)i / (float)(h > 1 ? h - 1 : 1);
        Uint8 cr = (Uint8)(top.r + t * ((float)bot.r - top.r));
        Uint8 cg = (Uint8)(top.g + t * ((float)bot.g - top.g));
        Uint8 cb = (Uint8)(top.b + t * ((float)bot.b - top.b));

        int inset = 0;
        if (i < rad) {
            int dy = rad - i;
            inset = rad - (int)std::sqrt((float)(rad * rad - dy * dy));
        } else if (i >= h - rad) {
            int dy = i - (h - 1 - rad);
            inset = rad - (int)std::sqrt((float)(rad * rad - dy * dy));
        }

        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_RenderDrawLine(r, x + inset, y + i, x + w - 1 - inset, y + i);
    }
}

static void draw_circle_filled(SDL_Renderer* r, int cx, int cy, int radius, PanelColor c) {
    set_color(r, c);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)std::sqrt((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

static void init_panel_fonts() {
    if (s_fontsAttempted) return;
    s_fontsAttempted = true;

    const char* fontPaths[] = {
        "assets/fonts/NotoSans-Regular.ttf",
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        nullptr
    };
    const char* boldPaths[] = {
        "assets/fonts/NotoSans-Bold.ttf",
        "assets/fonts/Roboto-Bold.ttf",
        "assets/fonts/DejaVuSans-Bold.ttf",
        "C:/Windows/Fonts/segoeuib.ttf",
        "C:/Windows/Fonts/consolab.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        nullptr
    };

    for (int i = 0; fontPaths[i]; i++) {
        if (!s_panelFont) s_panelFont = TTF_OpenFont(fontPaths[i], 13);
        if (!s_panelFontSmall) s_panelFontSmall = TTF_OpenFont(fontPaths[i], 11);
        if (s_panelFont && s_panelFontSmall) break;
    }
    for (int i = 0; boldPaths[i]; i++) {
        if (!s_panelFontBold) s_panelFontBold = TTF_OpenFont(boldPaths[i], 13);
        if (s_panelFontBold) break;
    }
    if (!s_panelFontBold && s_panelFont) {
        s_panelFontBold = s_panelFont;
    }
}

static SDL_Texture* render_text_texture(SDL_Renderer* r, TTF_Font* font,
                                         const std::string& text, PanelColor color,
                                         int* outW, int* outH) {
    if (!font || text.empty()) { *outW = 0; *outH = 0; return nullptr; }
    SDL_Color sc = {color.r, color.g, color.b, color.a};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), sc);
    if (!surf) { *outW = 0; *outH = 0; return nullptr; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    *outW = surf->w;
    *outH = surf->h;
    SDL_FreeSurface(surf);
    return tex;
}

static void draw_text(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                       int x, int y, PanelColor color) {
    int w, h;
    SDL_Texture* tex = render_text_texture(r, font, text, color, &w, &h);
    if (tex) {
        SDL_Rect dst = {x, y, w, h};
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

static void draw_text_right(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                              int rightX, int y, PanelColor color) {
    int w, h;
    SDL_Texture* tex = render_text_texture(r, font, text, color, &w, &h);
    if (tex) {
        SDL_Rect dst = {rightX - w, y, w, h};
        SDL_RenderCopy(r, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

static void draw_section_header(SDL_Renderer* r, const std::string& title,
                                 int x, int y, int w, int h, int rad, PanelColor accent) {
    draw_gradient_v_rounded(r, x, y, w, h, rad,
                    COL_HEADER_BG,
                    {(Uint8)(COL_HEADER_BG.r - 5), (Uint8)(COL_HEADER_BG.g - 5),
                     (Uint8)(COL_HEADER_BG.b - 5), 255});
    fill_rounded_rect(r, x, y + 2, 4, h - 4, 2, accent);
    draw_horizontal_line(r, x + rad, x + w - rad, y + h - 1, COL_SEPARATOR);
    if (s_panelFontBold) {
        draw_text(r, s_panelFontBold, title, x + 14, y + (h - 13) / 2, COL_TEXT_PRIMARY);
    }
}

static void draw_data_row_rounded(SDL_Renderer* r, const std::string& label, const std::string& value,
                                    int x, int y, int w, int h, bool alt, PanelColor dotColor,
                                    bool isFirst, bool isLast) {
    PanelColor bg = alt ? COL_BG_ROW_B : COL_BG_ROW_A;

    if (isFirst && isLast) {
        fill_rounded_rect(r, x, y, w, h, 5, bg);
    } else if (isFirst) {
        fill_rounded_rect(r, x, y, w, h + 10, 5, bg);
        fill_rect(r, x, y + h - 1, w, 1, bg);
    } else if (isLast) {
        fill_rect(r, x, y, w, 5, bg);
        fill_rounded_rect(r, x, y, w, h, 5, bg);
    } else {
        fill_rect(r, x, y, w, h, bg);
    }

    if (!isLast) {
        PanelColor sep = {COL_SEPARATOR.r, COL_SEPARATOR.g, COL_SEPARATOR.b, 80};
        draw_horizontal_line(r, x + 10, x + w - 10, y + h - 1, sep);
    }

    draw_circle_filled(r, x + 14, y + h / 2, 3, dotColor);

    if (s_panelFont) {
        draw_text(r, s_panelFont, label, x + 24, y + (h - 13) / 2, COL_TEXT_LABEL);
        draw_text_right(r, s_panelFont, value, x + w - 12, y + (h - 13) / 2, COL_TEXT_VALUE);
    }
}


static std::string float_to_str(float v, int decimals = 1) {
    char buf[64];
    if (decimals == 0) snprintf(buf, sizeof(buf), "%.0f", v);
    else if (decimals == 1) snprintf(buf, sizeof(buf), "%.1f", v);
    else snprintf(buf, sizeof(buf), "%.2f", v);
    return std::string(buf);
}

void render_sprite_panel(SDL_Renderer* renderer, const Sprite& sprite) {
    init_panel_fonts();

    int windowW, windowH;
    SDL_GetRendererOutputSize(renderer, &windowW, &windowH);

    int panelX = STAGE_X;
    int panelY = STAGE_Y + STAGE_HEIGHT + 4;
    int panelW = STAGE_WIDTH;
    int panelH = windowH - panelY - 4;

    if (panelH < 60) return;

    int mainRad = 8;

    fill_rounded_rect(renderer, panelX, panelY, panelW, panelH, mainRad, COL_BG_DARK);

    draw_gradient_v_rounded(renderer, panelX, panelY, panelW, 4, mainRad,
                    {COL_ACCENT_BLUE.r, COL_ACCENT_BLUE.g, COL_ACCENT_BLUE.b, 180},
                    {COL_ACCENT_BLUE.r, COL_ACCENT_BLUE.g, COL_ACCENT_BLUE.b, 40});

    draw_rounded_rect_outline(renderer, panelX, panelY, panelW, panelH, mainRad, COL_BORDER);

    int cx = panelX + 8;
    int cy = panelY + 8;
    int cw = panelW - 16;
    int sectionH = 24;
    int rowH = 24;
    int innerRad = 6;

    int thumbSize = 48;
    int headerH = thumbSize + 14;

    fill_rounded_rect(renderer, cx, cy, cw, headerH, innerRad, COL_BG_SECTION);
    draw_rounded_rect_outline(renderer, cx, cy, cw, headerH, innerRad, COL_BORDER);

    int thumbX = cx + 8;
    int thumbY = cy + (headerH - thumbSize) / 2;

    fill_rounded_rect(renderer, thumbX, thumbY, thumbSize, thumbSize, 6, COL_COORD_BG);
    draw_rounded_rect_outline(renderer, thumbX, thumbY, thumbSize, thumbSize, 6, COL_BORDER_LIGHT);

    if (sprite.texture) {
        int srcW = sprite.width;
        int srcH = sprite.height;
        float fitScale = 1.0f;
        if (srcW > 0 && srcH > 0) {
            float sx = (float)(thumbSize - 6) / (float)srcW;
            float sy = (float)(thumbSize - 6) / (float)srcH;
            fitScale = (sx < sy) ? sx : sy;
            if (fitScale > 1.0f) fitScale = 1.0f;
        }
        int dw = (int)(srcW * fitScale);
        int dh = (int)(srcH * fitScale);
        SDL_Rect dst = {thumbX + (thumbSize - dw) / 2, thumbY + (thumbSize - dh) / 2, dw, dh};
        SDL_RenderCopy(renderer, sprite.texture, nullptr, &dst);
    }

    int infoX = thumbX + thumbSize + 12;
    int infoY = cy + 8;

    std::string spriteName = sprite.name.empty() ? "Sprite" : sprite.name;
    if (s_panelFontBold) {
        draw_text(renderer, s_panelFontBold, spriteName, infoX, infoY, COL_TEXT_PRIMARY);
    }

    int badgeY = infoY + 20;

    PanelColor visBadgeBg = sprite.visible ? COL_ACCENT_GREEN : COL_ACCENT_RED;
    PanelColor visBadgeFg = {255, 255, 255, 255};
    std::string visText = sprite.visible ? "Visible" : "Hidden";
    int badgeW = 60, badgeH = 18;
    fill_rounded_rect(renderer, infoX, badgeY, badgeW, badgeH, badgeH / 2, visBadgeBg);
    if (s_panelFontSmall) {
        int tw, th;
        SDL_Texture* btex = render_text_texture(renderer, s_panelFontSmall, visText, visBadgeFg, &tw, &th);
        if (btex) {
            SDL_Rect bd = {infoX + (badgeW - tw) / 2, badgeY + (badgeH - th) / 2, tw, th};
            SDL_RenderCopy(renderer, btex, nullptr, &bd);
            SDL_DestroyTexture(btex);
        }
    }

    std::string sizeStr = float_to_str(sprite.scale * 100.0f, 0) + "%";
    if (s_panelFontSmall) {
        draw_text(renderer, s_panelFontSmall, sizeStr, infoX + badgeW + 10, badgeY + 2, COL_TEXT_SECONDARY);
    }

    std::string penText = sprite.isPenDown ? "Pen Down" : "Pen Up";
    PanelColor penBadgeBg = sprite.isPenDown ? COL_ACCENT_ORANGE : COL_BADGE_BG;
    PanelColor penBadgeFg = sprite.isPenDown ? PanelColor{255,255,255,255} : COL_TEXT_SECONDARY;
    int penBadgeX = infoX + badgeW + 64;
    int penBadgeW = 66;
    fill_rounded_rect(renderer, penBadgeX, badgeY, penBadgeW, badgeH, badgeH / 2, penBadgeBg);
    draw_rounded_rect_outline(renderer, penBadgeX, badgeY, penBadgeW, badgeH, badgeH / 2,
                               sprite.isPenDown ? COL_ACCENT_ORANGE : COL_BORDER_LIGHT);
    if (s_panelFontSmall) {
        int tw2, th2;
        SDL_Texture* ptex = render_text_texture(renderer, s_panelFontSmall, penText, penBadgeFg, &tw2, &th2);
        if (ptex) {
            SDL_Rect pd = {penBadgeX + (penBadgeW - tw2) / 2, badgeY + (badgeH - th2) / 2, tw2, th2};
            SDL_RenderCopy(renderer, ptex, nullptr, &pd);
            SDL_DestroyTexture(ptex);
        }
    }

    cy += headerH + 6;

    float scratchX = sprite.x - (STAGE_X + STAGE_WIDTH / 2.0f);
    float scratchY = (STAGE_Y + STAGE_HEIGHT / 2.0f) - sprite.y;

    int coordBoxH = 48;
    fill_rounded_rect(renderer, cx, cy, cw, coordBoxH, innerRad, COL_COORD_BG);
    draw_rounded_rect_outline(renderer, cx, cy, cw, coordBoxH, innerRad, COL_BORDER);

    int halfW = cw / 2;
    set_color(renderer, COL_SEPARATOR);
    SDL_RenderDrawLine(renderer, cx + halfW, cy + 8, cx + halfW, cy + coordBoxH - 8);

    if (s_panelFontSmall && s_panelFontBold) {
        int leftCol = cx + 16;
        int rightCol = cx + halfW + 16;

        fill_rounded_rect(renderer, leftCol - 4, cy + 6, 22, 16, 4, {COL_ACCENT_BLUE.r, COL_ACCENT_BLUE.g, COL_ACCENT_BLUE.b, 60});
        draw_text(renderer, s_panelFontSmall, "X", leftCol, cy + 7, COL_ACCENT_BLUE);
        draw_text(renderer, s_panelFontBold, float_to_str(scratchX, 1), leftCol + 24, cy + 6, COL_TEXT_PRIMARY);

        fill_rounded_rect(renderer, rightCol - 4, cy + 6, 22, 16, 4, {COL_ACCENT_GREEN.r, COL_ACCENT_GREEN.g, COL_ACCENT_GREEN.b, 60});
        draw_text(renderer, s_panelFontSmall, "Y", rightCol, cy + 7, COL_ACCENT_GREEN);
        draw_text(renderer, s_panelFontBold, float_to_str(scratchY, 1), rightCol + 24, cy + 6, COL_TEXT_PRIMARY);

        fill_rounded_rect(renderer, leftCol - 4, cy + 26, 30, 16, 4, {COL_ACCENT_PURPLE.r, COL_ACCENT_PURPLE.g, COL_ACCENT_PURPLE.b, 50});
        draw_text(renderer, s_panelFontSmall, "Dir", leftCol, cy + 27, COL_ACCENT_PURPLE);
        draw_text(renderer, s_panelFontBold, float_to_str(sprite.angle, 1) + "\xC2\xB0", leftCol + 32, cy + 26, COL_TEXT_PRIMARY);

        fill_rounded_rect(renderer, rightCol - 4, cy + 26, 36, 16, 4, {COL_ACCENT_ORANGE.r, COL_ACCENT_ORANGE.g, COL_ACCENT_ORANGE.b, 50});
        draw_text(renderer, s_panelFontSmall, "Size", rightCol, cy + 27, COL_ACCENT_ORANGE);
        draw_text(renderer, s_panelFontBold, float_to_str(sprite.scale * 100.0f, 0) + "%", rightCol + 38, cy + 26, COL_TEXT_PRIMARY);
    }

    cy += coordBoxH + 6;

    if (!sprite.sayText.empty()) {
        int sayBoxH = 30;
        fill_rounded_rect(renderer, cx, cy, cw, sayBoxH, innerRad, {50, 50, 80, 255});
        draw_rounded_rect_outline(renderer, cx, cy, cw, sayBoxH, innerRad, {80, 80, 140, 255});
        fill_rounded_rect(renderer, cx + 2, cy + 3, 4, sayBoxH - 6, 2, COL_ACCENT_PURPLE);
        if (s_panelFont) {
            std::string sayDisplay = "\xE2\x80\x9C" + sprite.sayText + "\xE2\x80\x9D";
            if (sayDisplay.length() > 50) sayDisplay = sayDisplay.substr(0, 47) + "...\xE2\x80\x9D";
            draw_text(renderer, s_panelFont, sayDisplay, cx + 14, cy + (sayBoxH - 13) / 2,
                      {200, 180, 255, 255});
        }
        cy += sayBoxH + 6;
    }

    struct SectionData {
        std::string title;
        PanelColor accent;
        std::vector<std::pair<std::string, std::string>> rows;
    };

    std::vector<SectionData> sections;

    {
        SectionData sec;
        sec.title = "COSTUME";
        sec.accent = COL_ACCENT_TEAL;
        std::string costumeInfo = "\xE2\x80\x94";
        if (!sprite.costumes.empty()) {
            int idx = sprite.currentCostumeIndex;
            if (idx >= 0 && idx < (int)sprite.costumes.size())
                costumeInfo = sprite.costumes[idx].name;
        }
        sec.rows.push_back({"Current", costumeInfo});
        sec.rows.push_back({"Total", std::to_string(sprite.costumes.size())});
        if (!sprite.costumes.empty()) {
            int idx = sprite.currentCostumeIndex;
            if (idx >= 0 && idx < (int)sprite.costumes.size()) {
                sec.rows.push_back({"Dimensions", std::to_string(sprite.costumes[idx].width) + " x " +
                                     std::to_string(sprite.costumes[idx].height)});
            }
        }
        sections.push_back(sec);
    }

    {
        SectionData sec;
        sec.title = "PEN";
        sec.accent = COL_ACCENT_ORANGE;
        sec.rows.push_back({"Status", sprite.isPenDown ? "Down" : "Up"});
        char penCol[32];
        snprintf(penCol, sizeof(penCol), "R:%d G:%d B:%d", sprite.penR, sprite.penG, sprite.penB);
        sec.rows.push_back({"Color", penCol});
        sec.rows.push_back({"Size", float_to_str((float)sprite.penSize, 1)});
        sections.push_back(sec);
    }

    if (!sprite.variables.empty()) {
        SectionData sec;
        sec.title = "VARIABLES";
        sec.accent = COL_ACCENT_PURPLE;
        for (size_t i = 0; i < sprite.variables.size(); i++) {
            sec.rows.push_back({sprite.variables[i].name, sprite.variables[i].value});
        }
        sections.push_back(sec);
    }

    {
        SectionData sec;
        sec.title = "DETAILS";
        sec.accent = COL_BORDER_LIGHT;
        sec.rows.push_back({"Engine", "Scratch SDL"});
        sec.rows.push_back({"Costumes", std::to_string(sprite.costumes.size())});
        sec.rows.push_back({"Variables", std::to_string(sprite.variables.size())});
        if (!sprite.sayText.empty()) {
            std::string st = sprite.sayText;
            if (st.length() > 20) st = st.substr(0, 17) + "...";
            sec.rows.push_back({"Saying", st});
        }
        sections.push_back(sec);
    }

    for (size_t s = 0; s < sections.size(); s++) {
        SectionData& sec = sections[s];

        if (cy + sectionH > panelY + panelH - 8) break;

        draw_section_header(renderer, sec.title, cx, cy, cw, sectionH, innerRad, sec.accent);
        cy += sectionH;

        for (size_t i = 0; i < sec.rows.size(); i++) {
            if (cy + rowH > panelY + panelH - 8) break;

            bool isFirst = (i == 0);
            bool isLast  = (i == sec.rows.size() - 1);
            bool alt     = (i % 2 == 1);

            draw_data_row_rounded(renderer, sec.rows[i].first, sec.rows[i].second,
                                   cx, cy, cw, rowH, alt, sec.accent, isFirst, isLast);
            cy += rowH;
        }

        if (s == 1 && !sprite.costumes.empty()) {
            int swatchY = cy - rowH * 2 + 4;
            int swatchX = cx + cw - 34;
            fill_rounded_rect(renderer, swatchX, swatchY, 16, 14, 3,
                              {sprite.penR, sprite.penG, sprite.penB, 255});
            draw_rounded_rect_outline(renderer, swatchX, swatchY, 16, 14, 3, COL_BORDER_LIGHT);
        }

        cy += 4;
    }

    int footerRemain = panelY + panelH - cy;
    if (footerRemain > 0) {
        fill_rounded_rect(renderer, cx, cy, cw, footerRemain - 8, innerRad, COL_BG_DARK);
    }

    draw_gradient_v_rounded(renderer, panelX, panelY + panelH - 4, panelW, 4, mainRad,
                    {COL_ACCENT_BLUE.r, COL_ACCENT_BLUE.g, COL_ACCENT_BLUE.b, 40},
                    {COL_ACCENT_BLUE.r, COL_ACCENT_BLUE.g, COL_ACCENT_BLUE.b, 160});
}
