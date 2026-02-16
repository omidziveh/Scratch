#include "costume_editor.h"
#include "../utils/logger.h"
#include <cmath>
#include <cstdio>

static void draw_point_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int x, int y, int size, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderTarget(renderer, canvas);
    filledCircleRGBA(renderer, x, y, size, r, g, b, 255);
    SDL_SetRenderTarget(renderer, nullptr);
}

static void draw_line_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int x1, int y1, int x2, int y2, int size, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderTarget(renderer, canvas);
    if (size <= 1) {
        lineRGBA(renderer, x1, y1, x2, y2, r, g, b, 255);
    } else {
        thickLineRGBA(renderer, x1, y1, x2, y2, size, r, g, b, 255);
    }
    SDL_SetRenderTarget(renderer, nullptr);
}

static void draw_rect_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderTarget(renderer, canvas);
    rectangleRGBA(renderer, x1, y1, x2, y2, r, g, b, 255);
    SDL_SetRenderTarget(renderer, nullptr);
}

static void draw_circle_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderTarget(renderer, canvas);
    circleRGBA(renderer, cx, cy, radius, r, g, b, 255);
    SDL_SetRenderTarget(renderer, nullptr);
}

static void erase_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int x, int y, int size) {
    SDL_SetRenderTarget(renderer, canvas);
    filledCircleRGBA(renderer, x, y, size, 255, 255, 255, 255);
    SDL_SetRenderTarget(renderer, nullptr);
}

static void flood_fill_on_canvas(SDL_Renderer* renderer, SDL_Texture* canvas, int x, int y, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderTarget(renderer, canvas);
    filledCircleRGBA(renderer, x, y, 20, r, g, b, 255);
    SDL_SetRenderTarget(renderer, nullptr);
}

void ceditor_init(CostumeEditor* editor, SDL_Renderer* renderer, int ex, int ey) {
    editor->is_open = false;
    editor->current_tool = CTOOL_PENCIL;
    editor->pen_r = 0;
    editor->pen_g = 0;
    editor->pen_b = 0;
    editor->pen_size = 3;
    editor->is_drawing = false;
    editor->last_x = 0;
    editor->last_y = 0;
    editor->start_x = 0;
    editor->start_y = 0;
    editor->undo_count = 0;
    editor->undo_index = -1;
    editor->target_costume_index = -1;
    editor->editor_x = ex;
    editor->editor_y = ey;
    editor->canvas_offset_x = ex + (CEDITOR_W - CEDITOR_CANVAS_SIZE) / 2;
    editor->canvas_offset_y = ey + 45;

    editor->canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CEDITOR_CANVAS_SIZE, CEDITOR_CANVAS_SIZE);
    SDL_SetTextureBlendMode(editor->canvas, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, editor->canvas);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    editor->preview_canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CEDITOR_CANVAS_SIZE, CEDITOR_CANVAS_SIZE);
    SDL_SetTextureBlendMode(editor->preview_canvas, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < CEDITOR_MAX_UNDO; i++) {
        editor->undo_stack[i] = nullptr;
    }

    log_info("Costume editor initialized");
}

void ceditor_destroy(CostumeEditor* editor) {
    if (editor->canvas) SDL_DestroyTexture(editor->canvas);
    if (editor->preview_canvas) SDL_DestroyTexture(editor->preview_canvas);
    for (int i = 0; i < CEDITOR_MAX_UNDO; i++) {
        if (editor->undo_stack[i]) SDL_DestroyTexture(editor->undo_stack[i]);
    }
    editor->canvas = nullptr;
    editor->preview_canvas = nullptr;
}

void ceditor_open(CostumeEditor* editor, int costume_index, SDL_Texture* source, SDL_Renderer* renderer) {
    editor->is_open = true;
    editor->target_costume_index = costume_index;
    editor->is_drawing = false;

    SDL_SetRenderTarget(renderer, editor->canvas);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    if (source) {
        SDL_RenderCopy(renderer, source, nullptr, nullptr);
    }
    SDL_SetRenderTarget(renderer, nullptr);

    for (int i = 0; i < CEDITOR_MAX_UNDO; i++) {
        if (editor->undo_stack[i]) {
            SDL_DestroyTexture(editor->undo_stack[i]);
            editor->undo_stack[i] = nullptr;
        }
    }
    editor->undo_count = 0;
    editor->undo_index = -1;
    ceditor_save_undo(editor, renderer);

    log_info("Costume editor opened for costume " + std::to_string(costume_index));
}

void ceditor_close(CostumeEditor* editor) {
    editor->is_open = false;
    editor->is_drawing = false;
    log_info("Costume editor closed");
}

void ceditor_save_undo(CostumeEditor* editor, SDL_Renderer* renderer) {
    if (editor->undo_count >= CEDITOR_MAX_UNDO) {
        if (editor->undo_stack[0]) SDL_DestroyTexture(editor->undo_stack[0]);
        for (int i = 0; i < CEDITOR_MAX_UNDO - 1; i++) {
            editor->undo_stack[i] = editor->undo_stack[i + 1];
        }
        editor->undo_stack[CEDITOR_MAX_UNDO - 1] = nullptr;
        editor->undo_count--;
    }

    SDL_Texture* snap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CEDITOR_CANVAS_SIZE, CEDITOR_CANVAS_SIZE);
    SDL_SetRenderTarget(renderer, snap);
    SDL_RenderCopy(renderer, editor->canvas, nullptr, nullptr);
    SDL_SetRenderTarget(renderer, nullptr);
    editor->undo_stack[editor->undo_count] = snap;
    editor->undo_index = editor->undo_count;
    editor->undo_count++;
}

void ceditor_undo(CostumeEditor* editor, SDL_Renderer* renderer) {
    if (editor->undo_index <= 0) return;
    editor->undo_index--;
    SDL_SetRenderTarget(renderer, editor->canvas);
    SDL_RenderCopy(renderer, editor->undo_stack[editor->undo_index], nullptr, nullptr);
    SDL_SetRenderTarget(renderer, nullptr);
    log_info("Costume editor: undo performed");
}

void ceditor_set_tool(CostumeEditor* editor, CDrawTool tool) {
    editor->current_tool = tool;
}

void ceditor_set_color(CostumeEditor* editor, Uint8 r, Uint8 g, Uint8 b) {
    editor->pen_r = r;
    editor->pen_g = g;
    editor->pen_b = b;
}

void ceditor_handle_mouse_down(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer) {
    if (!editor->is_open) return;

    int cx = mx - editor->canvas_offset_x;
    int cy = my - editor->canvas_offset_y;

    if (cx >= 0 && cx < CEDITOR_CANVAS_SIZE && cy >= 0 && cy < CEDITOR_CANVAS_SIZE) {
        editor->is_drawing = true;
        editor->last_x = cx;
        editor->last_y = cy;
        editor->start_x = cx;
        editor->start_y = cy;

        if (editor->current_tool == CTOOL_PENCIL) {
            draw_point_on_canvas(renderer, editor->canvas, cx, cy, editor->pen_size, editor->pen_r, editor->pen_g, editor->pen_b);
        } else if (editor->current_tool == CTOOL_ERASER) {
            erase_on_canvas(renderer, editor->canvas, cx, cy, editor->pen_size * 2);
        } else if (editor->current_tool == CTOOL_FILL) {
            flood_fill_on_canvas(renderer, editor->canvas, cx, cy, editor->pen_r, editor->pen_g, editor->pen_b);
            ceditor_save_undo(editor, renderer);
            editor->is_drawing = false;
        }
        return;
    }

    int tool_y = editor->editor_y + CEDITOR_H - 35;
    for (int i = 0; i < 6; i++) {
        int bx = editor->editor_x + 8 + i * 62;
        if (mx >= bx && mx <= bx + 56 && my >= tool_y && my <= tool_y + 24) {
            editor->current_tool = (CDrawTool)i;
            log_info("Costume editor: tool set to " + std::to_string(i));
            return;
        }
    }

    int undo_x = editor->editor_x + CEDITOR_W - 55;
    if (mx >= undo_x && mx <= undo_x + 48 && my >= tool_y && my <= tool_y + 24) {
        ceditor_undo(editor, renderer);
        return;
    }

    int close_x = editor->editor_x + CEDITOR_W - 22;
    int close_y = editor->editor_y + 5;
    if (mx >= close_x && mx <= close_x + 16 && my >= close_y && my <= close_y + 16) {
        ceditor_close(editor);
        return;
    }

    int color_y = editor->editor_y + 25;
    Uint8 preset_r[] = {0, 255, 0, 0, 255, 255, 128, 0};
    Uint8 preset_g[] = {0, 0, 255, 0, 255, 0, 0, 128};
    Uint8 preset_b[] = {0, 0, 0, 255, 0, 255, 128, 128};
    for (int i = 0; i < 8; i++) {
        int cpx = editor->editor_x + CEDITOR_W - 25;
        int cpy = color_y + i * 18;
        if (mx >= cpx && mx <= cpx + 14 && my >= cpy && my <= cpy + 14) {
            ceditor_set_color(editor, preset_r[i], preset_g[i], preset_b[i]);
            return;
        }
    }

    int size_y = editor->editor_y + CEDITOR_H - 60;
    int sizes[] = {1, 3, 5, 8, 12};
    for (int i = 0; i < 5; i++) {
        int sx = editor->editor_x + 8 + i * 40;
        if (mx >= sx && mx <= sx + 30 && my >= size_y && my <= size_y + 18) {
            editor->pen_size = sizes[i];
            return;
        }
    }
}

void ceditor_handle_mouse_move(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer) {
    if (!editor->is_open || !editor->is_drawing) return;

    int cx = mx - editor->canvas_offset_x;
    int cy = my - editor->canvas_offset_y;
    if (cx < 0) cx = 0;
    if (cx >= CEDITOR_CANVAS_SIZE) cx = CEDITOR_CANVAS_SIZE - 1;
    if (cy < 0) cy = 0;
    if (cy >= CEDITOR_CANVAS_SIZE) cy = CEDITOR_CANVAS_SIZE - 1;

    if (editor->current_tool == CTOOL_PENCIL) {
        draw_line_on_canvas(renderer, editor->canvas, editor->last_x, editor->last_y, cx, cy, editor->pen_size, editor->pen_r, editor->pen_g, editor->pen_b);
        editor->last_x = cx;
        editor->last_y = cy;
    } else if (editor->current_tool == CTOOL_ERASER) {
        draw_line_on_canvas(renderer, editor->canvas, editor->last_x, editor->last_y, cx, cy, editor->pen_size * 2, 255, 255, 255);
        editor->last_x = cx;
        editor->last_y = cy;
    }
}

void ceditor_handle_mouse_up(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer) {
    if (!editor->is_open || !editor->is_drawing) return;

    int cx = mx - editor->canvas_offset_x;
    int cy = my - editor->canvas_offset_y;
    if (cx < 0) cx = 0;
    if (cx >= CEDITOR_CANVAS_SIZE) cx = CEDITOR_CANVAS_SIZE - 1;
    if (cy < 0) cy = 0;
    if (cy >= CEDITOR_CANVAS_SIZE) cy = CEDITOR_CANVAS_SIZE - 1;

    if (editor->current_tool == CTOOL_LINE) {
        draw_line_on_canvas(renderer, editor->canvas, editor->start_x, editor->start_y, cx, cy, editor->pen_size, editor->pen_r, editor->pen_g, editor->pen_b);
    } else if (editor->current_tool == CTOOL_RECT) {
        draw_rect_on_canvas(renderer, editor->canvas, editor->start_x, editor->start_y, cx, cy, editor->pen_r, editor->pen_g, editor->pen_b);
    } else if (editor->current_tool == CTOOL_CIRCLE) {
        int dx = cx - editor->start_x;
        int dy = cy - editor->start_y;
        int radius = (int)sqrt((double)(dx * dx + dy * dy));
        draw_circle_on_canvas(renderer, editor->canvas, editor->start_x, editor->start_y, radius, editor->pen_r, editor->pen_g, editor->pen_b);
    }

    editor->is_drawing = false;
    ceditor_save_undo(editor, renderer);
}

void ceditor_handle_key(CostumeEditor* editor, SDL_Keycode key, SDL_Renderer* renderer) {
    if (!editor->is_open) return;
    if (key == SDLK_z && (SDL_GetModState() & KMOD_CTRL)) {
        ceditor_undo(editor, renderer);
    }
    if (key == SDLK_1) ceditor_set_tool(editor, CTOOL_PENCIL);
    if (key == SDLK_2) ceditor_set_tool(editor, CTOOL_ERASER);
    if (key == SDLK_3) ceditor_set_tool(editor, CTOOL_LINE);
    if (key == SDLK_4) ceditor_set_tool(editor, CTOOL_RECT);
    if (key == SDLK_5) ceditor_set_tool(editor, CTOOL_CIRCLE);
    if (key == SDLK_6) ceditor_set_tool(editor, CTOOL_FILL);
}

SDL_Texture* ceditor_get_result(CostumeEditor* editor) {
    return editor->canvas;
}

void ceditor_render(CostumeEditor* editor, SDL_Renderer* renderer) {
    if (!editor->is_open) return;

    SDL_Rect panel = {editor->editor_x, editor->editor_y, CEDITOR_W, CEDITOR_H};
    SDL_SetRenderDrawColor(renderer, 248, 248, 250, 255);
    SDL_RenderFillRect(renderer, &panel);
    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(renderer, &panel);

    stringRGBA(renderer, editor->editor_x + 10, editor->editor_y + 8, "Costume Editor", 30, 30, 30, 255);

    SDL_Rect close_btn = {editor->editor_x + CEDITOR_W - 22, editor->editor_y + 5, 16, 16};
    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);
    SDL_RenderFillRect(renderer, &close_btn);
    stringRGBA(renderer, close_btn.x + 3, close_btn.y + 2, "X", 255, 255, 255, 255);

    SDL_Rect canvas_bg = {editor->canvas_offset_x - 1, editor->canvas_offset_y - 1, CEDITOR_CANVAS_SIZE + 2, CEDITOR_CANVAS_SIZE + 2};
    for (int row = 0; row < CEDITOR_CANVAS_SIZE; row += 16) {
        for (int col = 0; col < CEDITOR_CANVAS_SIZE; col += 16) {
            bool light = ((row / 16 + col / 16) % 2 == 0);
            SDL_SetRenderDrawColor(renderer, light ? 230 : 200, light ? 230 : 200, light ? 230 : 200, 255);
            SDL_Rect sq = {editor->canvas_offset_x + col, editor->canvas_offset_y + row, 16, 16};
            SDL_RenderFillRect(renderer, &sq);
        }
    }

    SDL_Rect canvas_rect = {editor->canvas_offset_x, editor->canvas_offset_y, CEDITOR_CANVAS_SIZE, CEDITOR_CANVAS_SIZE};
    SDL_RenderCopy(renderer, editor->canvas, nullptr, &canvas_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &canvas_bg);

    const char* tool_names[] = {"Pencil", "Eraser", "Line", "Rect", "Circle", "Fill"};
    Uint8 tool_r[] = {66, 234, 251, 52, 171, 255};
    Uint8 tool_g[] = {133, 67, 188, 168, 71, 152};
    Uint8 tool_b[] = {244, 53, 4, 83, 188, 0};
    int tool_y = editor->editor_y + CEDITOR_H - 35;

    for (int i = 0; i < 6; i++) {
        int bx = editor->editor_x + 8 + i * 62;
        SDL_Rect btn = {bx, tool_y, 56, 24};
        if ((int)editor->current_tool == i) {
            SDL_SetRenderDrawColor(renderer, tool_r[i], tool_g[i], tool_b[i], 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        }
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderDrawRect(renderer, &btn);
        stringRGBA(renderer, bx + 4, tool_y + 6, tool_names[i], 255, 255, 255, 255);
    }

    int undo_x = editor->editor_x + CEDITOR_W - 55;
    SDL_Rect undo_btn = {undo_x, tool_y, 48, 24};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &undo_btn);
    stringRGBA(renderer, undo_x + 8, tool_y + 6, "Undo", 255, 255, 255, 255);

    int size_y = editor->editor_y + CEDITOR_H - 60;
    int sizes[] = {1, 3, 5, 8, 12};
    for (int i = 0; i < 5; i++) {
        int sx = editor->editor_x + 8 + i * 40;
        SDL_Rect sbtn = {sx, size_y, 30, 18};
        if (editor->pen_size == sizes[i]) {
            SDL_SetRenderDrawColor(renderer, 66, 133, 244, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        }
        SDL_RenderFillRect(renderer, &sbtn);
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderDrawRect(renderer, &sbtn);
        char sz_text[8];
        snprintf(sz_text, sizeof(sz_text), "%d", sizes[i]);
        stringRGBA(renderer, sx + 8, size_y + 4, sz_text, 255, 255, 255, 255);
    }

    stringRGBA(renderer, editor->editor_x + 220, size_y + 4, "Size:", 80, 80, 80, 255);

    Uint8 preset_r[] = {0, 255, 0, 0, 255, 255, 128, 0};
    Uint8 preset_g[] = {0, 0, 255, 0, 255, 0, 0, 128};
    Uint8 preset_b[] = {0, 0, 0, 255, 0, 255, 128, 128};
    int color_y = editor->editor_y + 25;
    for (int i = 0; i < 8; i++) {
        int cpx = editor->editor_x + CEDITOR_W - 25;
        int cpy = color_y + i * 18;
        SDL_Rect cswatch = {cpx, cpy, 14, 14};
        SDL_SetRenderDrawColor(renderer, preset_r[i], preset_g[i], preset_b[i], 255);
        SDL_RenderFillRect(renderer, &cswatch);
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer, &cswatch);
    }

    SDL_Rect cur_color = {editor->editor_x + CEDITOR_W - 45, color_y, 16, 16};
    SDL_SetRenderDrawColor(renderer, editor->pen_r, editor->pen_g, editor->pen_b, 255);
    SDL_RenderFillRect(renderer, &cur_color);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &cur_color);

    char tool_info[64];
    snprintf(tool_info, sizeof(tool_info), "Tool: %s  |  Size: %d", tool_names[(int)editor->current_tool], editor->pen_size);
    stringRGBA(renderer, editor->editor_x + 10, editor->editor_y + 28, tool_info, 100, 100, 100, 255);
}
