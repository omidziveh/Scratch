#include "frontend/draw.h"
#include "frontend/input.h"
#include "common/definitions.h"
#include "common/globals.h"
#include "backend/runtime.h"
#include "backend/logger.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

using namespace BlockCoding;

namespace BlockCoding {
    std::vector<Block*> all_blocks;
}

int main(int argc, char* argv[]) {
    init_logger();
    log_info("Starting application");

    GraphicsContext ctx;
    init_graphics(&ctx);

    // tester
    Block* b1 = new Block;
    b1->id = 1;
    b1->type = CMD_MOVE;
    b1->x = 100;
    b1->y = 100;
    b1->width = BLOCK_WIDTH;
    b1->height = BLOCK_HEIGHT;
    b1->args.push_back("20");
    all_blocks.push_back(b1);

    Block* b2 = new Block;
    b2->id = 2;
    b2->type = CMD_TURN;
    b2->x = 100;
    b2->y = 150;
    b2->width = BLOCK_WIDTH;
    b2->height = BLOCK_HEIGHT;
    b2->args.push_back("45");
    all_blocks.push_back(b2);

    Block* b3 = new Block;
    b3->id = 3;
    b3->type = CMD_WAIT;
    b3->x = 100;
    b3->y = 200;
    b3->width = BLOCK_WIDTH;
    b3->height = BLOCK_HEIGHT;
    b3->args.push_back("0.5");
    all_blocks.push_back(b3);

    b1->next = b2;
    b2->next = b3;

    Sprite playerSprite;
    playerSprite.x = WINDOW_WIDTH / 2.0f;
    playerSprite.y = WINDOW_HEIGHT / 2.0f;
    playerSprite.angle = 0;

    Runtime runtime;
    runtime_init(&runtime, b1, &playerSprite);

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
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (runtime.state == RUNTIME_RUNNING) {
                            runtime_stop(&runtime);
                        } else {
                            runtime_reset(&runtime);
                            runtime_start(&runtime);
                        }
                        break;
                    case SDLK_p:
                        if (runtime.state == RUNTIME_RUNNING) {
                            runtime_pause(&runtime);
                        } else if (runtime.state == RUNTIME_PAUSED) {
                            runtime_resume(&runtime);
                        }
                        break;
                    case SDLK_s:
                        runtime_set_step_mode(&runtime, !runtime.stepMode);
                        break;
                    case SDLK_n:
                        if (runtime.state == RUNTIME_PAUSED || runtime.stepMode) {
                            runtime_step(&runtime);
                        }
                        break;
                    case SDLK_r:
                        runtime_reset(&runtime);
                        playerSprite.x = WINDOW_WIDTH / 2.0f;
                        playerSprite.y = WINDOW_HEIGHT / 2.0f;
                        playerSprite.angle = 0;
                        break;
                    case SDLK_d:
                        log_info(runtime_get_status(&runtime));
                        break;
                }
            }
        }

        if (runtime.state == RUNTIME_RUNNING) {
            runtime_tick(&runtime);
        }

        clear(&ctx, 240, 240, 240);

        for (Block* b : all_blocks) {
            draw_block(b);

            if (b == runtime.currentBlock && runtime.state == RUNTIME_RUNNING) {
                SDL_SetRenderDrawColor(ctx.renderer, 0, 255, 0, 255);
                SDL_Rect highlight = {
                    (int)b->x - 2, (int)b->y - 2,
                    BLOCK_WIDTH + 4, BLOCK_HEIGHT + 4
                };
                SDL_RenderDrawRect(ctx.renderer, &highlight);
            }
        }

        SDL_SetRenderDrawColor(ctx.renderer, 255, 100, 100, 255);
        SDL_Rect spriteRect = {
            (int)playerSprite.x - 15, (int)playerSprite.y - 15,
            30, 30
        };
        SDL_RenderFillRect(ctx.renderer, &spriteRect);

        SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, 255);
        float rad = playerSprite.angle * 3.14159265f / 180.0f;
        int endX = (int)(playerSprite.x + 20 * std::cos(rad));
        int endY = (int)(playerSprite.y + 20 * std::sin(rad));
        SDL_RenderDrawLine(ctx.renderer,
                          (int)playerSprite.x, (int)playerSprite.y,
                          endX, endY);

        SDL_RenderPresent(ctx.renderer);
        SDL_Delay(16);
    }

    for (Block* b : all_blocks) {
        delete b;
    }

    shutdown_graphics(&ctx);
    close_logger();
    return 0;
}
