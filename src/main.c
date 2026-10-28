#include "../include/ui.h"
#include "../include/window.h"
#include <stdio.h>

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    UiState state;
    SDL_Event event;
    int running = 1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (init_window(&window, &renderer) != 0) {
        terminate(window, renderer);
        return 1;
    }

    ui_initialize(&state, renderer);
    SDL_StartTextInput();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            } else {
                ui_handle_event(&state, &event);
            }
        }

        ui_render(renderer, &state);
        SDL_Delay(1000 / 60);
    }
    
    ui_destroy(&state);  
    SDL_StopTextInput();
    terminate(window, renderer);
    return 0;
}
