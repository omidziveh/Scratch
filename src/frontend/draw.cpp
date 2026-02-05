#include "draw.h"

namespace BlockCoding {
    static SDL_Renderer* g_renderer = nullptr;
    
    void init_graphics(GraphicsContext* ctx) {
        SDL_Init(SDL_INIT_VIDEO);
        ctx->window = SDL_CreateWindow(
            "Block Coding System",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
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
        SDL_Rect rect = {b->x, b->y, BLOCK_WIDTH, BLOCK_HEIGHT};
        
        if (b->type == BLOCK_MOVE || b->type == BLOCK_TURN || b->type == BLOCK_GOTO) {
            SDL_SetRenderDrawColor(g_renderer, 70, 130, 180, 255);  
        }
        else if (b->type == BLOCK_REPEAT || b->type == BLOCK_IF || b->type == BLOCK_WAIT) {
            SDL_SetRenderDrawColor(g_renderer, 255, 200, 50, 255); 
        }
        
        SDL_RenderFillRect(g_renderer, &rect);
    }
    
    void shutdown_graphics(GraphicsContext* ctx) {
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
        SDL_Quit();
    }
    
    void draw_snap_preview(Block* dragging, Block* target) {
        if (!dragging || !target) return;
        
        SDL_SetRenderDrawColor(g_renderer, 100, 200, 100, 150);
        
        int x1 = dragging->x + 60;
        int y1 = dragging->y;
        int x2 = target->x + 60;
        int y2 = target->y + BLOCK_HEIGHT;
        
        SDL_RenderDrawLine(g_renderer, x1, y1, x2, y2);
    }
}
