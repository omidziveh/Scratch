#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "gfx/SDL2_gfxPrimitives.h"

std::vector<Block*> all_blocks;

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Block Coding",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Stage stage;
    Sprite sprite;
    sprite.texture = load_texture(renderer, "assets/cat.png");

    init_palette();

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    handle_mouse_down(event.button.x, event.button.y);
                    break;
                case SDL_MOUSEMOTION:
                    handle_mouse_motion(event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    handle_mouse_up(event.button.x, event.button.y);
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        draw_stage_background(renderer, &stage);
        draw_stage_border(renderer, &stage);
        draw_sprite(renderer, &sprite, &stage);
        draw_palette(renderer);

        for (Block* b : all_blocks) {
            roundedBoxRGBA(renderer,
                (int)b->x, (int)b->y,
                (int)(b->x + b->width), (int)(b->y + b->height),
                BLOCK_CORNER_RADIUS, 66, 133, 244, 255);
        }

        SDL_RenderPresent(renderer);
    }

    for (Block* b : all_blocks) {
        delete b;
    }
    all_blocks.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
