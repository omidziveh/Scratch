#include "draw.h"
#include "../common/globals.h"

namespace BlockCoding {

    static SDL_Renderer* g_renderer = nullptr;

    void init_graphics(GraphicsContext* ctx) {
        SDL_Init(SDL_INIT_VIDEO);
        ctx->window = SDL_CreateWindow(
            "Block Coding",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
        );
        ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
        g_renderer = ctx->renderer;
    }

    void clear(GraphicsContext* ctx, int r, int g, int b) {
        SDL_SetRenderDrawColor(ctx->renderer, r, g, b, 255);
        SDL_RenderClear(ctx->renderer);
    }

    void draw_block(Block* b) {
        if (!b) return;

        SDL_Rect rect = {
            (int)b->x, (int)b->y,
            BLOCK_WIDTH, BLOCK_HEIGHT
        };

        switch (b->type) {
            case CMD_MOVE:
            case CMD_TURN:
            case CMD_GOTO:
                SDL_SetRenderDrawColor(g_renderer, 70, 130, 180, 255);
                break;
            case CMD_REPEAT:
            case CMD_IF:
            case CMD_WAIT:
                SDL_SetRenderDrawColor(g_renderer, 255, 200, 50, 255);
                break;
            case CMD_SAY:
                SDL_SetRenderDrawColor(g_renderer, 148, 92, 213, 255);
                break;
            case CMD_EVENT_CLICK:
                SDL_SetRenderDrawColor(g_renderer, 255, 220, 100, 255);
                break;
            default:
                SDL_SetRenderDrawColor(g_renderer, 128, 128, 128, 255);
                break;
        }

        SDL_RenderFillRect(g_renderer, &rect);
        SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255);
        SDL_RenderDrawRect(g_renderer, &rect);
    }

    void shutdown_graphics(GraphicsContext* ctx) {
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
    }

    void draw_snap_preview(Block* dragging, Block* target) {
        if (!dragging || !target) return;

        SDL_SetRenderDrawColor(g_renderer, 100, 200, 100, 150);
        int x1 = (int)dragging->x + 60;
        int y1 = (int)dragging->y;
        int x2 = (int)target->x + 60;
        int y2 = (int)target->y + BLOCK_HEIGHT;
        SDL_RenderDrawLine(g_renderer, x1, y1, x2, y2);
    }

}
