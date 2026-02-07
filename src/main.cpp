#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    
    SDL_Window* window = SDL_CreateWindow(
        "Scratch Clone - Sprite Rendering",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    Stage stage;
    stage.x = STAGE_X;
    stage.y = STAGE_Y;
    stage.width = STAGE_WIDTH;
    stage.height = STAGE_HEIGHT;
    stage.border_color.r = 100;
    stage.border_color.g = 100;
    stage.border_color.b = 100;
    stage.border_color.a = 255;
    stage.background_color.r = 255;
    stage.background_color.g = 255;
    stage.background_color.b = 255;
    stage.background_color.a = 255;
    
    Sprite cat;
    cat.x = STAGE_WIDTH / 2;
    cat.y = STAGE_HEIGHT / 2;
    cat.width = 80;
    cat.height = 80;
    cat.angle = 0;
    cat.visible = 1;
    cat.texture = load_texture(renderer, "assets/sprite.png");
    
    int running = 1;
    SDL_Event event;
    
    while (running == 1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    cat.angle = cat.angle - 15;
                }
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    cat.angle = cat.angle + 15;
                }
                if (event.key.keysym.sym == SDLK_UP) {
                    cat.y = cat.y - 10;
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    cat.y = cat.y + 10;
                }
                if (event.key.keysym.sym == SDLK_a) {
                    cat.x = cat.x - 10;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    cat.x = cat.x + 10;
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);
        
        draw_stage_background(renderer, &stage);
        draw_stage_border(renderer, &stage);
        draw_sprite(renderer, &cat, &stage);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    SDL_DestroyTexture(cat.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}
