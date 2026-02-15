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
#include "frontend/menu.h"
#include "frontend/hover.h"
#include "utils/logger.h"
#include "frontend/text_input.h"
#include "utils/system_logger.h"
#include "backend/block_executor_looks.h"
#include "backend/sound.h"
#include "backend/runtime.h"
#include "frontend/pen.h"
#include <set>
#include "backend/logic.h"

Sprite sprite;
Runtime gRuntime;
Stage stage;

void init_program(SDL_Renderer& renderer) {
    syslog_init();
    menu_init();
    pen_init(&renderer);
    init_logger("debug.log");
    log_info("Application started");
    sound_load("meow", "../assets/meow.wav");
    sprite.texture = load_texture(&renderer, "../assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png - sprite will be invisible");
    }
}




int main(int argc, char* argv[]) {
    int g_execution_index = -1;
    bool g_is_executing = false;
    bool g_step_mode = false;
    bool g_waiting_for_step = false;

    // ==========================================================
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (!sound_init()) {
        log_warning("Sound engine failed to initialize");
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
    // ==========================================================

    init_program(*renderer);

    {
        const char* costume_files[] = {"../assets/cat.png", "../assets/cat2.png"};
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

    int palette_scroll_offset = 0;
    int palette_max_scroll = get_palette_total_height(palette_items) - PALETTE_HEIGHT + 20;
    if (palette_max_scroll < 0) palette_max_scroll = 0;

    std::vector<Block> blocks;
    int next_block_id = 1;


    Stage stage;
    stage.renderer = renderer;

    sprite.x = STAGE_X + STAGE_WIDTH / 2;
    sprite.y = STAGE_Y + STAGE_HEIGHT / 2;

    TextInputState text_state;

    int mouse_x = 0, mouse_y = 0;
    bool hover_run = false;
    bool hover_stop = false;

    bool running = true;
    SDL_Event event;

    std::vector<Runtime> activeRuntimes;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEWHEEL: 
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);

                    if (mx >= PALETTE_X && mx < PALETTE_X + PALETTE_WIDTH &&
                        my >= PALETTE_Y && my < PALETTE_Y + PALETTE_HEIGHT) {

                        palette_scroll_offset -= event.wheel.y * 30;

                        if (palette_scroll_offset < 0) palette_scroll_offset = 0;
                        if (palette_scroll_offset > palette_max_scroll) palette_scroll_offset = palette_max_scroll;
                    }
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
                            my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + 5 + 30) {
                            activeRuntimes.clear();
                            for (Block& b : blocks) {
                                if (b.type == CMD_START && b.next) {
                                    Runtime rt;
                                    runtime_init(&rt, b.next, &sprite);
                                    runtime_start(&rt);
                                    activeRuntimes.push_back(rt);
                                }
                            }
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 50 && mx <= TOOLBAR_WIDTH - 50 + 30 &&
                            my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + 5 + 30) {
                                for (Runtime& rt : activeRuntimes) {
                                    runtime_stop(&rt);
                                }
                                activeRuntimes.clear();
                            break;
                        }

                        bool clicked_arg = false;
                        for (auto& block : blocks) {
                            if (try_click_arg(block, mx, my, text_state)) {
                                clicked_arg = true;
                                break;
                            }
                        }

                        if (!clicked_arg) {
                            if (text_state.active) {
                                commit_editing(text_state, blocks);
                            }
                            handle_mouse_down(event, blocks, palette_items, next_block_id, palette_scroll_offset);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        menu_handle_mouse_up(event.button.x, event.button.y);
                        handle_mouse_up(event, blocks);
                    }
                    break;

                case SDL_MOUSEMOTION: {
                    int mx = event.motion.x;
                    int my = event.motion.y;
                    mouse_x = mx;
                    mouse_y = my;

                    menu_handle_mouse_move(mx, my);

                    hover_run = (mx >= TOOLBAR_WIDTH - 90 && mx <= TOOLBAR_WIDTH - 90 + 30 &&
                                 my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + 5 + 30);

                    hover_stop = (mx >= TOOLBAR_WIDTH - 50 && mx <= TOOLBAR_WIDTH - 50 + 30 &&
                                  my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + 5 + 30);

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
                        if (event.key.keysym.sym == SDLK_F12) {
                            for (Runtime& rt : activeRuntimes) {
                                rt.stepMode = !rt.stepMode;
                                rt.waitingForStep = rt.stepMode;
                            }
                        }
                        if (event.key.keysym.sym == SDLK_SPACE) {
                            if (g_step_mode && g_waiting_for_step) {
                                g_waiting_for_step = false;
                            }
                        }
                    }
                    break;
            }
        }


        tick_cursor(text_state);
        logger_tick();

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        for (Runtime& rt : activeRuntimes) {
            runtime_tick(&rt, &stage, mouseX, mouseY);
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        draw_toolbar(renderer);
        draw_palette(renderer, palette_items, palette_scroll_offset);
        draw_coding_area(renderer);
        draw_stage(renderer, sprite);

        pen_render(renderer);

        for (const auto& block : blocks) {
            std::string label = block_get_label(block.type);
            draw_block_glow(renderer, block);
            draw_block(renderer, block, label);
            draw_arg_boxes(renderer, block, text_state);
        }


        if (hover_run) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
            SDL_Rect run_rect = { TOOLBAR_WIDTH - 90, TOOLBAR_Y + 5, 30, 30 };
            SDL_RenderFillRect(renderer, &run_rect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        if (hover_stop) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            SDL_Rect stop_rect = { TOOLBAR_WIDTH - 50, TOOLBAR_Y + 5, 30, 30 };
            SDL_RenderFillRect(renderer, &stop_rect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        if (syslog_is_visible()) {
            syslog_render(renderer);
        }

        
        menu_render(renderer);
        
        render_palette_hover(renderer, palette_items, mouse_x, mouse_y, palette_scroll_offset);
        SDL_RenderPresent(renderer);
    }

    pen_shutdown();

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
    sound_cleanup();

    return 0;
}
