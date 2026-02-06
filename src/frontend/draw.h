#pragma once
#include <SDL2/SDL.h>
#include "../common/definitions.h"

namespace BlockCoding {

    struct GraphicsContext {
        SDL_Window* window;
        SDL_Renderer* renderer;
    };

    void init_graphics(GraphicsContext* ctx);
    void shutdown_graphics(GraphicsContext* ctx);
    void clear(GraphicsContext* ctx, int r, int g, int b);
    void draw_block(Block* b);
    void draw_snap_preview(Block* dragging, Block* target);

}
