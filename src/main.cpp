#include "src/frontend/draw.h"
#include "src/frontend/input.h"
#include "src/common/definitions.h"
#include <SDL2/SDL.h>
#include <vector>

using namespace BlockCoding;

std::vector<Block*> all_blocks;

int main(int argc, char* argv[]) {
    GraphicsContext ctx;
    init_graphics(&ctx);
    
    Block* b1 = new Block;
    b1->type = BLOCK_MOVE;
    b1->x = 100;
    b1->y = 100;
    b1->next = nullptr;
    all_blocks.push_back(b1);
    
    Block* b2 = new Block;
    b2->type = BLOCK_REPEAT;
    b2->x = 300;
    b2->y = 200;
    b2->next = nullptr;
    all_blocks.push_back(b2);
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                handle_mouse_down(event.button.x, event.button.y);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                handle_mouse_motion(event.motion.x, event.motion.y);
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                handle_mouse_up(event.button.x, event.button.y);
            }
        }
        
        clear(&ctx, 240, 240, 240);
        
        for (Block* b : all_blocks) {
            draw_block(b);
        }
        
        SDL_RenderPresent(ctx.renderer);
        SDL_Delay(16);  
    }
    
    for (Block* b : all_blocks) {
        delete b;
    }
    shutdown_graphics(&ctx);
    
    return 0;
}
