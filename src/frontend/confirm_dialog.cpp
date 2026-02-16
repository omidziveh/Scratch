#include "confirm_dialog.h"
#include <cstring>

void cdialog_init(ConfirmDialog* dialog, int screen_w, int screen_h) {
    dialog->is_open = false;
    dialog->result = CDLG_NONE;
    dialog->screen_w = screen_w;
    dialog->screen_h = screen_h;
    memset(dialog->title, 0, sizeof(dialog->title));
    memset(dialog->message, 0, sizeof(dialog->message));
}

void cdialog_show(ConfirmDialog* dialog, const char* title, const char* msg) {
    dialog->is_open = true;
    dialog->result = CDLG_NONE;
    strncpy(dialog->title, title, 63);
    dialog->title[63] = '\0';
    strncpy(dialog->message, msg, 127);
    dialog->message[127] = '\0';
}

void cdialog_close(ConfirmDialog* dialog) {
    dialog->is_open = false;
    dialog->result = CDLG_NONE;
}

void cdialog_render(ConfirmDialog* dialog, SDL_Renderer* renderer) {
    if (!dialog->is_open) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlay = {0, 0, dialog->screen_w, dialog->screen_h};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_RenderFillRect(renderer, &overlay);

    int dx = (dialog->screen_w - CDIALOG_W) / 2;
    int dy = (dialog->screen_h - CDIALOG_H) / 2;

    SDL_Rect box = {dx, dy, CDIALOG_W, CDIALOG_H};
    SDL_SetRenderDrawColor(renderer, 252, 252, 252, 255);
    SDL_RenderFillRect(renderer, &box);

    SDL_Rect shadow = {dx + 3, dy + 3, CDIALOG_W, CDIALOG_H};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawColor(renderer, 252, 252, 252, 255);
    SDL_RenderFillRect(renderer, &box);

    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(renderer, &box);

    SDL_Rect title_bar = {dx, dy, CDIALOG_W, 32};
    SDL_SetRenderDrawColor(renderer, 66, 133, 244, 255);
    SDL_RenderFillRect(renderer, &title_bar);
    stringRGBA(renderer, dx + 12, dy + 9, dialog->title, 255, 255, 255, 255);

    stringRGBA(renderer, dx + 20, dy + 55, dialog->message, 50, 50, 50, 255);

    SDL_Rect yes_btn = {dx + 30, dy + CDIALOG_H - 42, 80, 28};
    SDL_SetRenderDrawColor(renderer, 76, 175, 80, 255);
    SDL_RenderFillRect(renderer, &yes_btn);
    stringRGBA(renderer, yes_btn.x + 25, yes_btn.y + 8, "Yes", 255, 255, 255, 255);

    SDL_Rect no_btn = {dx + 120, dy + CDIALOG_H - 42, 80, 28};
    SDL_SetRenderDrawColor(renderer, 244, 67, 54, 255);
    SDL_RenderFillRect(renderer, &no_btn);
    stringRGBA(renderer, no_btn.x + 28, no_btn.y + 8, "No", 255, 255, 255, 255);

    SDL_Rect cancel_btn = {dx + 210, dy + CDIALOG_H - 42, 80, 28};
    SDL_SetRenderDrawColor(renderer, 158, 158, 158, 255);
    SDL_RenderFillRect(renderer, &cancel_btn);
    stringRGBA(renderer, cancel_btn.x + 14, cancel_btn.y + 8, "Cancel", 255, 255, 255, 255);
}

CDialogResult cdialog_handle_click(ConfirmDialog* dialog, int mx, int my) {
    if (!dialog->is_open) return CDLG_NONE;

    int dx = (dialog->screen_w - CDIALOG_W) / 2;
    int dy = (dialog->screen_h - CDIALOG_H) / 2;

    if (mx >= dx + 30 && mx <= dx + 110 && my >= dy + CDIALOG_H - 42 && my <= dy + CDIALOG_H - 14) {
        dialog->is_open = false;
        dialog->result = CDLG_YES;
        return CDLG_YES;
    }

    if (mx >= dx + 120 && mx <= dx + 200 && my >= dy + CDIALOG_H - 42 && my <= dy + CDIALOG_H - 14) {
        dialog->is_open = false;
        dialog->result = CDLG_NO;
        return CDLG_NO;
    }

    if (mx >= dx + 210 && mx <= dx + 290 && my >= dy + CDIALOG_H - 42 && my <= dy + CDIALOG_H - 14) {
        dialog->is_open = false;
        dialog->result = CDLG_CANCEL;
        return CDLG_CANCEL;
    }

    return CDLG_NONE;
}
