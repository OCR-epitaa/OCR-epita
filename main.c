#include "../include/button.h"
#include "../include/window.h"
#include "SDL2/SDL.h"
#include <stdio.h>

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    init_window(&window, &renderer);

    SDL_Rect button = { .x = 350, .y = 250, .w = 100, .h = 100 };

    int x_c, y_c;
    SDL_Event event;

    int hovered;
    int clicked;

    while (1) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                terminate(window, renderer);
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {

            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    terminate(window, renderer);
                    return 0;
                }
            }
        }

        SDL_GetMouseState(&x_c, &y_c);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        if (is_inside(&button, x_c, y_c)) {
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            if (handle_button(button, event))
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        }

        SDL_RenderFillRect(renderer, &button);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 24);
    }

    terminate(window, renderer);

    return 0;
}
