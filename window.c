#include "../include/window.h"
#include "SDL2/SDL.h"
#include <stdio.h>

void init_window(SDL_Window **window, SDL_Renderer **renderer)
{
    SDL_Init(SDL_INIT_VIDEO);
    *window = SDL_CreateWindow("Projection", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (*window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC );
    if (*renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
    }
    
    
}

void terminate(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}



