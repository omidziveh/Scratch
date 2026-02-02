#include "draw.h"
#include <iostream>
using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init_graphics() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Error: " << SDL_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow(
        "Block Coding System",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        cout << "Window Error: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        cout << "Renderer Error: " << SDL_GetError() << endl;
        return false;
    }

    return true;
}

void shutdown_graphics() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void clear(int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
}

void present() {
    SDL_RenderPresent(renderer);
}

void set_color(int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void fill_rect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void draw_block(Block* b) {
    if (b == nullptr) {
        return;
    }

    Color block_color;

    if (b->type == MOVE || b->type == TURN || b->type == GOTO) {
        block_color = COLOR_MOTION;
    }
    else if (b->type == REPEAT || b->type == IF || b->type == WAIT) {
        block_color = COLOR_CONTROL;
    }
    else {
        block_color = COLOR_GRAY;
    }

    set_color(block_color.r, block_color.g, block_color.b);

    fill_rect(b->x, b->y, b->width, b->height);

    set_color(
        block_color.r * 0.7,
        block_color.g * 0.7,
        block_color.b * 0.7
    );
    
    SDL_Rect border = {b->x, b->y, b->width, b->height};
    SDL_RenderDrawRect(renderer, &border);
}
