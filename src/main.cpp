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
#include "frontend/pen.h"

int main(int argc, char* argv[]) {
    int g_execution_index = -1;
    bool g_is_executing = false;
    bool g_step_mode = false;
    bool g_waiting_for_step = false;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
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

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    pen_init(renderer);

    init_logger("debug.log");
    log_info("Application started");

    sound_load("meow", "../assets/meow.wav");

    Sprite sprite;
    sprite.texture = load_texture(renderer, "../assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png - sprite will be invisible");
    }

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

                case SDL_MOUSEWHEEL: {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);

                    if (mx >= PALETTE_X && mx < PALETTE_X + PALETTE_WIDTH &&
                        my >= PALETTE_Y && my < PALETTE_Y + PALETTE_HEIGHT) {

                        palette_scroll_offset -= event.wheel.y * 30;

                        if (palette_scroll_offset < 0) palette_scroll_offset = 0;
                        if (palette_scroll_offset > palette_max_scroll) palette_scroll_offset = palette_max_scroll;
                    }
                    break;
                }

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
        logger_tick();

        if (g_is_executing && g_execution_index >= 0 && g_execution_index < (int)blocks.size()) {
            if (g_step_mode && g_waiting_for_step) {

            } else {
                for (auto& b : blocks) {
                    b.is_running = false;
                }

                blocks[g_execution_index].is_running = true;
                blocks[g_execution_index].glow_start_time = SDL_GetTicks();
                Block& current = blocks[g_execution_index];
                current.is_running = true;
                current.glow_start_time = SDL_GetTicks();

                ExecutionContext exec_ctx;
                exec_ctx.sprite = &sprite;

                float oldX = sprite.x;
                float oldY = sprite.y;

                switch (current.type) {

                    case CMD_MOVE: {
                        float steps = 10.0f;
                        if (!current.args.empty())
                            steps = (float)std::atof(current.args[0].c_str());
                        float rad = sprite.angle * (float)M_PI / 180.0f;
                        sprite.x += steps * std::cos(rad);
                        sprite.y += steps * std::sin(rad);
                        break;
                    }

                    case CMD_TURN: {
                        float deg = 15.0f;
                        if (!current.args.empty())
                            deg = (float)std::atof(current.args[0].c_str());
                        sprite.angle += deg;
                        break;
                    }

                    case CMD_GOTO: {
                        float gx = 0.0f, gy = 0.0f;
                        if (current.args.size() >= 1) gx = (float)std::atof(current.args[0].c_str());
                        if (current.args.size() >= 2) gy = (float)std::atof(current.args[1].c_str());
                        sprite.x = STAGE_X + STAGE_WIDTH / 2.0f + gx;
                        sprite.y = STAGE_Y + STAGE_HEIGHT / 2.0f - gy;
                        break;
                    }

                    case CMD_SET_X: {
                        float nx = 0.0f;
                        if (!current.args.empty()) nx = (float)std::atof(current.args[0].c_str());
                        sprite.x = STAGE_X + STAGE_WIDTH / 2.0f + nx;
                        break;
                    }

                    case CMD_SET_Y: {
                        float ny = 0.0f;
                        if (!current.args.empty()) ny = (float)std::atof(current.args[0].c_str());
                        sprite.y = STAGE_Y + STAGE_HEIGHT / 2.0f - ny;
                        break;
                    }

                    case CMD_CHANGE_X: {
                        float dx = 0.0f;
                        if (!current.args.empty()) dx = (float)std::atof(current.args[0].c_str());
                        sprite.x += dx;
                        break;
                    }

                    case CMD_CHANGE_Y: {
                        float dy = 0.0f;
                        if (!current.args.empty()) dy = (float)std::atof(current.args[0].c_str());
                        sprite.y -= dy;
                        break;
                    }

                    case CMD_SWITCH_COSTUME:
                    case CMD_NEXT_COSTUME:
                    case CMD_SET_SIZE:
                    case CMD_CHANGE_SIZE:
                    case CMD_SHOW:
                    case CMD_HIDE:
                        execute_looks_block(&current, exec_ctx);
                        break;

                    case CMD_PLAY_SOUND:
                        if (!current.args.empty())
                            play_sound(current.args[0], exec_ctx.sprite->volume);
                        break;

                
                    case CMD_PEN_DOWN:
                        sprite.isPenDown = 1;
                        sprite.prevPenX = sprite.x;
                        sprite.prevPenY = sprite.y;
                        break;

                    case CMD_PEN_UP:
                        sprite.isPenDown = 0;
                        break;

                    case CMD_PEN_CLEAR:
                        pen_clear(renderer);
                        break;

                    case CMD_PEN_SET_COLOR: {
                        if (current.args.size() >= 3) {
                            sprite.penR = (Uint8)std::atoi(current.args[0].c_str());
                            sprite.penG = (Uint8)std::atoi(current.args[1].c_str());
                            sprite.penB = (Uint8)std::atoi(current.args[2].c_str());
                            pen_set_color(sprite.penR, sprite.penG, sprite.penB);
                        }
                        break;
                    }

                    case CMD_PEN_SET_SIZE: {
                        if (!current.args.empty()) {
                            sprite.penSize = std::atoi(current.args[0].c_str());
                            if (sprite.penSize < 1) sprite.penSize = 1;
                            pen_set_size(sprite.penSize);
                        }
                        break;
                    }

                    case CMD_PEN_STAMP:
                        pen_stamp(renderer, sprite);
                        break;


                    default:
                        break;
                }

                // --- CHANGED: رسم خط Pen بعد از حرکت ---
                if (sprite.isPenDown && (sprite.x != oldX || sprite.y != oldY)) {
                    pen_draw_line(renderer, oldX, oldY, sprite.x, sprite.y, sprite);
                }

                pen_update(renderer, sprite);

                sprite.prevPenX = sprite.x;
                sprite.prevPenY = sprite.y;

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
