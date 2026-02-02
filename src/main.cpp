#include "src/frontend/draw.h"
#include "src/common/definitions.h"
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    if (!init_graphics()) {
        return -1;
    }

    Block move_block;
    move_block.type = MOVE;
    move_block.x = 100;
    move_block.y = 100;
    move_block.width = BLOCK_WIDTH;
    move_block.height = BLOCK_HEIGHT;

    Block repeat_block;
    repeat_block.type = REPEAT;
    repeat_block.x = 100;
    repeat_block.y = 200;
    repeat_block.width = BLOCK_WIDTH;
    repeat_block.height = BLOCK_HEIGHT;

    Block turn_block;
    turn_block.type = TURN;
    turn_block.x = 100;
    turn_block.y = 300;
    turn_block.width = BLOCK_WIDTH;
    turn_block.height = BLOCK_HEIGHT;

    Block if_block;
    if_block.type = IF;
    if_block.x = 350;
    if_block.y = 100;
    if_block.width = BLOCK_WIDTH;
    if_block.height = BLOCK_HEIGHT;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        clear(50, 50, 50);
        draw_block(&move_block);
        draw_block(&repeat_block);
        draw_block(&turn_block);
        draw_block(&if_block);

        present();
        SDL_Delay(16);
    }

    shutdown_graphics();
    return 0;
}
