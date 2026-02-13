#include <SDL2/SDL.h>
#ifdef __linux__
#include <SDL2/SDL_image.h>
#else
#include <SDL_image.h>
#endif
#include <iostream>
#include <vector>
#include <cmath>
#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "frontend/block_utils.h"
#include "utils/logger.h"
#include "frontend/text_input.h"
#include "utils/system_logger.h"

int main(int argc, char* argv[]) {
    int g_execution_index = -1;
    bool g_is_executing = false;
    bool g_step_mode = false;
    bool g_waiting_for_step = false;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    syslog_init();

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
    sprite.texture = load_texture(renderer, "../assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png — sprite will be invisible");
    }

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

                        // --- Green Flag (Run) button ---
                        if (mx >= TOOLBAR_WIDTH - 90 && mx <= TOOLBAR_WIDTH - 90 + 30 &&
                            my >= 5 && my <= 5 + 30) {
                            g_execution_index = 0;
                            g_is_executing = true;
                            break;
                        }

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
                    } else {
                        if (event.key.keysym.sym == SDLK_SPACE) {
                            if (g_step_mode && g_waiting_for_step) {
                                g_waiting_for_step = false;
                            }
                        }
                        if (event.key.keysym.sym == SDLK_F12) {
                            g_step_mode = !g_step_mode;
                            g_waiting_for_step = g_step_mode;
                        }
                    }
                    break;
                case SDLK_l:
                    syslog_toggle();
                    break;

            }
        }

        tick_cursor(text_state);

        // ===== STEP-BY-STEP EXECUTION =====
        if (g_is_executing && g_execution_index >= 0 && g_execution_index < (int)blocks.size()) {
            if (g_step_mode && g_waiting_for_step) {
                // Wait for Space key press
            } else {
                for (auto& b : blocks) {
                    b.is_running = false;
                }

                blocks[g_execution_index].is_running = true;
                blocks[g_execution_index].glow_start_time = SDL_GetTicks();

                g_execution_index++;

                if (g_execution_index >= (int)blocks.size()) {
                    blocks[g_execution_index - 1].is_running = false;
                    g_execution_index = -1;
                    g_is_executing = false;
                }

                if (g_step_mode) {
                    g_waiting_for_step = true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        draw_toolbar(renderer);
        draw_palette(renderer, palette_items);
        draw_coding_area(renderer);
        draw_stage(renderer, sprite);

        for (const auto& block : blocks) {
            std::string label = block_get_label(block.type);
            draw_block_glow(renderer, block);
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
