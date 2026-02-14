#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "frontend/block_utils.h"
#include "frontend/menu.h"
#include "frontend/hover.h"
#include "utils/logger.h"
#include "frontend/text_input.h"
#include "utils/system_logger.h"
#include "backend/block_executor_looks.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    syslog_init();
    menu_init();

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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    init_logger("debug.log");
    log_info("Application started");

    Sprite sprite;
    sprite.texture = load_texture(renderer, "assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png - sprite will be invisible");
    }

    {
        const char* costume_files[] = {"assets/cat.png", "assets/cat2.png"};
        const char* costume_names[] = {"costume1", "costume2"};
        int num_costumes = 2;

        for (int i = 0; i < num_costumes; i++) {
            SDL_Texture* tex = load_texture(renderer, costume_files[i]);
            if (tex) {
                int w = 0, h = 0;
                SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
                sprite.costumes.push_back(Costume(costume_names[i], tex, w, h));
            }
        }

        if (!sprite.costumes.empty()) {
            sprite.currentCostumeIndex = 0;
            sprite.texture = sprite.costumes[0].texture;
            sprite.width = sprite.costumes[0].width;
            sprite.height = sprite.costumes[0].height;
        }
    }

    std::vector<PaletteItem> palette_items;
    init_palette(palette_items);

    std::vector<Block> blocks;
    int next_block_id = 1;

    TextInputState text_state;

    int mouse_x = 0, mouse_y = 0;
    bool hover_run = false;
    bool hover_stop = false;

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

                        if (my < MENU_BAR_OFFSET) {
                            menu_handle_mouse_down(mx, my);
                            break;
                        }

                        if (menu_is_any_open()) {
                            menu_handle_mouse_down(mx, my);
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 90 && mx <= TOOLBAR_WIDTH - 90 + 30 &&
                            my >= 5 && my <= 5 + 30) {
                            g_execution_index = 0;
                            g_is_executing = true;
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 50 && mx <= TOOLBAR_WIDTH - 50 + 30 &&
                            my >= 5 && my <= 5 + 30) {
                            g_is_executing = false;
                            g_execution_index = -1;
                            for (auto& b : blocks) {
                                b.is_running = false;
                            }
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
                        menu_handle_mouse_up(event.button.x, event.button.y);
                        handle_mouse_up(event, blocks);
                    }
                    break;

                case SDL_MOUSEMOTION:
                {
                    int mx = event.motion.x;
                    int my = event.motion.y;
                    mouse_x = mx;
                    mouse_y = my;

                    menu_handle_mouse_move(mx, my);

                    hover_run = (mx >= TOOLBAR_WIDTH - 90 && mx <= TOOLBAR_WIDTH - 90 + 30 &&
                                 my >= 5 && my <= 5 + 30);

                    hover_stop = (mx >= TOOLBAR_WIDTH - 50 && mx <= TOOLBAR_WIDTH - 50 + 30 &&
                                  my >= 5 && my <= 5 + 30);

                    handle_mouse_motion(event, blocks);
                }
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
                        if (event.key.keysym.sym == SDLK_l) {
                            syslog_toggle();
                        }
                    }
                    break;
            }
        }

        tick_cursor(text_state);

        if (g_is_executing && g_execution_index >= 0 && g_execution_index < (int)blocks.size()) {
            for (auto& b : blocks) {
                b.is_running = false;
            }

            Block& current = blocks[g_execution_index];
            current.is_running = true;
            current.glow_start_time = SDL_GetTicks();

            ExecutionContext exec_ctx;
            exec_ctx.sprite = &sprite;

            switch (current.type) {
                case CMD_SWITCH_COSTUME:
                case CMD_NEXT_COSTUME:
                case CMD_SET_SIZE:
                case CMD_CHANGE_SIZE:
                case CMD_SHOW:
                case CMD_HIDE:
                    execute_looks_block(&current, exec_ctx);
                    break;
                default:
                    break;
            }

            g_execution_index++;

            if (g_execution_index >= (int)blocks.size()) {
                current.is_running = false;
                g_execution_index = -1;
                g_is_executing = false;
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

        render_palette_hover(renderer, palette_items, mouse_x, mouse_y);

        if (hover_run) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_Rect run_rect = { TOOLBAR_WIDTH - 90, 5, 30, 30 };
            SDL_RenderFillRect(renderer, &run_rect);
        }

        if (hover_stop) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_Rect stop_rect = { TOOLBAR_WIDTH - 50, 5, 30, 30 };
            SDL_RenderFillRect(renderer, &stop_rect);
        }

        if (syslog_is_visible()) {
            syslog_render(renderer);
        }

        menu_render(renderer);

        SDL_RenderPresent(renderer);
    }

    for (auto& c : sprite.costumes) {
        if (c.texture) {
            SDL_DestroyTexture(c.texture);
        }
    }
    close_logger();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
