#ifndef CONFIRM_DIALOG_H
#define CONFIRM_DIALOG_H
#include "../common/definitions.h" 

#include <SDL2/SDL.h>
#include "../gfx/SDL2_gfxPrimitives.h"

#define CDIALOG_W 320
#define CDIALOG_H 150

enum CDialogResult {
    CDLG_NONE = 0,
    CDLG_YES,
    CDLG_NO,
    CDLG_CANCEL
};

struct ConfirmDialog {
    bool is_open;
    char title[64];
    char message[128];
    CDialogResult result;
    int screen_w;
    int screen_h;
};

void cdialog_init(ConfirmDialog* dialog, int screen_w, int screen_h);
void cdialog_show(ConfirmDialog* dialog, const char* title, const char* msg);
void cdialog_close(ConfirmDialog* dialog);
void cdialog_render(ConfirmDialog* dialog, SDL_Renderer* renderer);
CDialogResult cdialog_handle_click(ConfirmDialog* dialog, int mx, int my);

#endif
