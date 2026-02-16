#ifndef COSTUME_EDITOR_H
#define COSTUME_EDITOR_H

#include <SDL2/SDL.h>
#include <string>
#include "../gfx/SDL2_gfxPrimitives.h"

#define CEDITOR_CANVAS_SIZE 256
#define CEDITOR_MAX_UNDO 20
#define CEDITOR_W 420
#define CEDITOR_H 380

enum CDrawTool {
    CTOOL_PENCIL = 0,
    CTOOL_ERASER,
    CTOOL_LINE,
    CTOOL_RECT,
    CTOOL_CIRCLE,
    CTOOL_FILL
};

struct CostumeEditor {
    bool is_open;
    SDL_Texture* canvas;
    SDL_Texture* preview_canvas;
    SDL_Texture* undo_stack[CEDITOR_MAX_UNDO];
    int undo_count;
    int undo_index;
    CDrawTool current_tool;
    Uint8 pen_r;
    Uint8 pen_g;
    Uint8 pen_b;
    int pen_size;
    bool is_drawing;
    int last_x;
    int last_y;
    int start_x;
    int start_y;
    int editor_x;
    int editor_y;
    int canvas_offset_x;
    int canvas_offset_y;
    int target_costume_index;
};

void ceditor_init(CostumeEditor* editor, SDL_Renderer* renderer, int ex, int ey);
void ceditor_destroy(CostumeEditor* editor);
void ceditor_open(CostumeEditor* editor, int costume_index, SDL_Texture* source, SDL_Renderer* renderer);
void ceditor_close(CostumeEditor* editor);
void ceditor_render(CostumeEditor* editor, SDL_Renderer* renderer);
void ceditor_handle_mouse_down(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer);
void ceditor_handle_mouse_move(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer);
void ceditor_handle_mouse_up(CostumeEditor* editor, int mx, int my, SDL_Renderer* renderer);
void ceditor_handle_key(CostumeEditor* editor, SDL_Keycode key, SDL_Renderer* renderer);
void ceditor_set_tool(CostumeEditor* editor, CDrawTool tool);
void ceditor_set_color(CostumeEditor* editor, Uint8 r, Uint8 g, Uint8 b);
void ceditor_save_undo(CostumeEditor* editor, SDL_Renderer* renderer);
void ceditor_undo(CostumeEditor* editor, SDL_Renderer* renderer);
SDL_Texture* ceditor_get_result(CostumeEditor* editor);

#endif
