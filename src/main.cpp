#include <SDL2/SDL.h>
#include <cstdio>
#include <vector>
#include <string>

#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "frontend/logger.h"
#include "backend/file_io.h"
#include "backend/memory.h"
#include "frontend/block_utils.h"

std::vector<Block> blocks;
Sprite sprite;
Stage stage;
int next_block_id = 1;
bool running = true;
int dragging_index = -1;
int drag_offset_x = 0;
int drag_offset_y = 0;

static Block* vector_to_chain(const std::vector<Block>& vec) {
    Block* head = nullptr;
    Block* tail = nullptr;
    for (size_t i = 0; i < vec.size(); i++) {
        Block* b = create_block(vec[i].type);
        b->id     = vec[i].id;
        b->x      = vec[i].x;
        b->y      = vec[i].y;
        b->width  = vec[i].width;
        b->height = vec[i].height;
        b->args   = vec[i].args;
        if (!head) {
            head = b;
            tail = b;
        } else {
            tail->next = b;
            tail = b;
        }
    }
    return head;
}

static void chain_to_vector(Block* head, std::vector<Block>& vec, int& max_id) {
    vec.clear();
    max_id = 0;
    Block* cur = head;
    while (cur) {
        Block b;
        b.id     = cur->id;
        b.type   = cur->type;
        b.x      = cur->x;
        b.y      = cur->y;
        b.width  = cur->width;
        b.height = cur->height;
        b.args   = cur->args;
        b.next   = nullptr;
        b.inner  = nullptr;
        vec.push_back(b);
        if (cur->id > max_id) max_id = cur->id;
        cur = cur->next;
    }
}


int main(int argc, char* argv[]) {
    init_logger("debug.log");
    log_info("Application starting...");

    GraphicsContext ctx;
    init_graphics(&ctx);
    if (!ctx.window || !ctx.renderer) {
        log_error("Failed to initialize graphics!");
        close_logger();
        return 1;
    }
    log_success("Graphics initialized");

    sprite.x = STAGE_X + STAGE_WIDTH / 2;
    sprite.y = STAGE_Y + STAGE_HEIGHT / 2;
    sprite.width  = 50;
    sprite.height = 50;
    sprite.angle  = 0;
    sprite.visible = true;
    sprite.isPenDown = false;
    sprite.currentCostumeIndex = 0;

    std::vector<PaletteItem> palette_items;
    init_palette(palette_items);
    log_info("Palette initialized with " + std::to_string(palette_items.size()) + " items");

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

                        int palette_hit = check_palette_click(mx, my, palette_items);
                        if (palette_hit >= 0) {
                            Block new_block;
                            new_block.id     = next_block_id++;
                            new_block.type   = palette_items[palette_hit].type;
                            new_block.x      = mx;
                            new_block.y      = my;
                            new_block.width  = palette_items[palette_hit].width;
                            new_block.height = palette_items[palette_hit].height;
                            new_block.next   = nullptr;
                            new_block.inner  = nullptr;

                            switch (new_block.type) {
                                case CMD_MOVE:  new_block.args.push_back("10");  break;
                                case CMD_TURN:  new_block.args.push_back("15");  break;
                                case CMD_WAIT:  new_block.args.push_back("1");   break;
                                case CMD_REPEAT: new_block.args.push_back("10"); break;
                                case CMD_SAY:
                                    new_block.args.push_back("Hello!");
                                    new_block.args.push_back("2");
                                    break;
                                default: break;
                            }

                            blocks.push_back(new_block);
                            dragging_index = (int)blocks.size() - 1;
                            drag_offset_x = 0;
                            drag_offset_y = 0;
                            log_info("Created block #" + std::to_string(new_block.id) +
                                     " type=" + blocktype_to_string(new_block.type));
                        } else {
                            handle_mouse_down(mx, my, blocks, dragging_index,
                                              drag_offset_x, drag_offset_y);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        handle_mouse_up(blocks, dragging_index);
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (dragging_index >= 0) {
                        handle_mouse_move(event.motion.x, event.motion.y,
                                          blocks, dragging_index,
                                          drag_offset_x, drag_offset_y);
                    }
                    break;

                case SDL_KEYDOWN:
          
                    if (event.key.keysym.sym == SDLK_s &&
                        (event.key.keysym.mod & KMOD_CTRL)) {
                        Block* chain = vector_to_chain(blocks);
                        if (save_to_file(chain, "project.blk")) {
                            save_sprite(sprite, "project.sprite");
                            log_success("Project saved!");
                        } else {
                            log_error("Failed to save project!");
                        }
                        delete_chain(chain);
                    }
       
                    if (event.key.keysym.sym == SDLK_o &&
                        (event.key.keysym.mod & KMOD_CTRL)) {
                        Block* chain = load_from_file("project.blk");
                        if (chain) {
                            int max_id = 0;
                            chain_to_vector(chain, blocks, max_id);
                            next_block_id = max_id + 1;
                            sprite = load_sprite("project.sprite");
                            delete_chain(chain);
                            log_success("Project loaded! (" +
                                        std::to_string(blocks.size()) + " blocks)");
                        } else {
                            log_error("Failed to load project!");
                        }
                    }
       
                    if (event.key.keysym.sym == SDLK_z &&
                        (event.key.keysym.mod & KMOD_CTRL)) {
                        if (!blocks.empty()) {
                            log_info("Removed block #" +
                                     std::to_string(blocks.back().id));
                            blocks.pop_back();
                        }
                    }
              
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;

                default:
                    break;
            }
        }

        clear(&ctx, 240, 240, 240);

        SDL_Rect stage_rect = { stage.x, stage.y, stage.width, stage.height };
        SDL_SetRenderDrawColor(ctx.renderer,
            stage.background_color.r, stage.background_color.g,
            stage.background_color.b, stage.background_color.a);
        SDL_RenderFillRect(ctx.renderer, &stage_rect);
        SDL_SetRenderDrawColor(ctx.renderer,
            stage.border_color.r, stage.border_color.g,
            stage.border_color.b, stage.border_color.a);
        SDL_RenderDrawRect(ctx.renderer, &stage_rect);

        if (sprite.visible) {
            SDL_Rect sp_rect = {
                (int)(sprite.x - sprite.width / 2),
                (int)(sprite.y - sprite.height / 2),
                sprite.width,
                sprite.height
            };
            SDL_SetRenderDrawColor(ctx.renderer, 0, 120, 255, 255);
            SDL_RenderFillRect(ctx.renderer, &sp_rect);
        }

        draw_palette(ctx.renderer, palette_items);

        for (size_t i = 0; i < blocks.size(); i++) {
            draw_block(ctx.renderer, blocks[i]);
        }

        SDL_RenderPresent(ctx.renderer);
        SDL_Delay(16); 
    }

    log_info("Shutting down...");
    shutdown_graphics(&ctx);
    close_logger();

    return 0;
}
