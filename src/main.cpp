#include "frontend/draw.h"
#include "common/definitions.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

int main() {
    
    cout << "=== Block Coding System ===" << endl;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL Init Failed: " << SDL_GetError() << endl;
        return 1;
    }

    // Initialize graphics
    if (!init_graphics()) {
        cerr << "Graphics Init Failed!" << endl;
        SDL_Quit();
        return 1;
    }
    
    cout << "Graphics initialized successfully" << endl;

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running) {
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                cout << "Window close requested" << endl;
            }
            else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
                cout << "ESC pressed - exiting" << endl;
            }
        }

        clear(240, 240, 240);
        set_color(100, 150, 255);
        fill_rect(100, 100, 200, 150);
        set_color(255, 100, 100);
        fill_rect(400, 100, 200, 150);
        set_color(100, 255, 100);
        fill_rect(700, 100, 200, 150);
        render();
        this_thread::sleep_for(chrono::milliseconds(16));
    }
    shutdown_graphics();
    cout << "Application closed" << endl;
    
    return 0;
}
