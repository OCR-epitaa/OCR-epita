#include "../include/window.h"
#include <stdio.h>

int init_window(SDL_Window **window, SDL_Renderer **renderer)
{
    *window = SDL_CreateWindow("Word Search OCR",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
        WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return -1;
    }

    *renderer = SDL_CreateRenderer(*window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*renderer == NULL)
        *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE);
    if (*renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        *window = NULL;
        SDL_Quit();
        return -1;
    }

    return 0;
}

void terminate(SDL_Window *window, SDL_Renderer *renderer)
{
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    if (window != NULL)
        SDL_DestroyWindow(window);
    SDL_Quit();
}
