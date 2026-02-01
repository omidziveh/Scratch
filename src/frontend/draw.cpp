#include "draw.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init_graphics() {
    window = SDL_CreateWindow("Block Coding", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN);
    
    if (!window) return false;
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr;
}

void shutdown_graphics() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void clear(int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
}

void render() {
    SDL_RenderPresent(renderer);
}

void set_color(int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void fill_rect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_rect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &rect);
}
