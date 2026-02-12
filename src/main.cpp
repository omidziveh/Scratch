#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>

#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "frontend/block_utils.h"
#include "utils/logger.h"
#include "frontend/text_input.h"
#include "backend/runtime.h"
#include "backend/memory.h"
#include "backend/logic.h"
#include "backend/sensing.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Block Coding",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    init_logger("debug.log");
    log_info("Application started");

    Sprite sprite;
    sprite.texture = load_texture(renderer, "assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png — sprite will be invisible");
    }

    // === RUNTIME TEST ============================================
    Stage stage;   // uses defaults from definitions.h
    Sprite testSprite = sprite;   // copy main sprite (or create new)

    // Build a simple program: repeat 4 times { move 50, turn 90 }
    Block* head = create_block(CMD_REPEAT);
    head->args[0] = "4";
    Block* moveBlock = create_block(CMD_MOVE);
    moveBlock->args[0] = "50";
    Block* turnBlock = create_block(CMD_TURN);
    turnBlock->args[0] = "90";

    connect_blocks(moveBlock, turnBlock);
    head->inner = moveBlock;   // repeat inner chain

    Runtime rt;
    runtime_init(&rt, head, &testSprite);
    runtime_start(&rt);

    log_separator();
    log_info("=== BEGIN RUNTIME TEST ===");
    for (int i = 0; i < 100 && rt.state == RUNTIME_RUNNING; ++i) {
        runtime_tick(&rt, &stage);
        log_debug("Tick " + std::to_string(i) +
                " | pos: (" + std::to_string(testSprite.x) + ", " +
                std::to_string(testSprite.y) + ") angle: " +
                std::to_string(testSprite.angle));
        if (rt.watchdogTriggered) break;
    }
    log_info("=== RUNTIME TEST FINISHED ===");
    log_separator();

    delete_chain(head);
    // =============================================================

    std::vector<PaletteItem> palette_items;
    init_palette(palette_items);

    std::vector<Block> blocks;
    int next_block_id = 1;

    TextInputState text_state;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x;
                        int my = event.button.y;

                        bool clicked_arg = false;
                        for (auto& block : blocks) {
                            int arg_idx = try_click_arg(block, mx, my);
                            if (arg_idx >= 0) {
                                if (text_state.active) {
                                    commit_editing(text_state, blocks);
                                }
                                begin_editing(text_state, block, arg_idx);
                                clicked_arg = true;
                                break;
                            }
                        }

                        if (!clicked_arg) {
                            if (text_state.active) {
                                commit_editing(text_state, blocks);
                            }
                            handle_mouse_down(event, blocks, palette_items, next_block_id);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        handle_mouse_up(event, blocks);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    handle_mouse_motion(event, blocks);
                    break;

                case SDL_TEXTINPUT:
                    if (text_state.active) {
                        on_text_input(text_state, event.text.text);
                    }
                    break;

                case SDL_KEYDOWN:
                    if (text_state.active) {
                        on_key_input(text_state, event.key.keysym.sym, blocks);
                    }
                    break;
            }
        }

        tick_cursor(text_state);

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        draw_toolbar(renderer);
        draw_palette(renderer, palette_items);
        draw_coding_area(renderer);
        draw_stage(renderer, sprite);

        for (const auto& block : blocks) {
            std::string label = block_get_label(block.type);
            draw_block(renderer, block, label);
            draw_arg_boxes(renderer, block, text_state);
        }

        SDL_RenderPresent(renderer);
    }

    if (sprite.texture) {
        SDL_DestroyTexture(sprite.texture);
    }
    close_logger();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
