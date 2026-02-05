#ifndef DRAW_H
#define DRAW_H

#include "../common/definitions.h"
#include <SDL2/SDL.h>

namespace BlockCoding {
    struct GraphicsContext {
        SDL_Window* window;
        SDL_Renderer* renderer;
    };
    
    void init_graphics(GraphicsContext* ctx);
    void clear(GraphicsContext* ctx, int r, int g, int b);
    void draw_block(Block* b);
    void shutdown_graphics(GraphicsContext* ctx);
    
    void draw_snap_preview(Block* dragging, Block* target);
}

#endif
