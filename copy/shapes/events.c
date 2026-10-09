#include "shapes.h"
#include "SDL2/SDL.h"
#include <stdio.h>

int handle_events(SDL_Rect *rectangle)
{
    SDL_Event event;

    while (SDL_PollEvent(&event) == 1) {
        if (event.type == SDL_QUIT) {
            return 0;
        }
        if (event.type == SDL_KEYDOWN) {

            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    return 0;
                case SDLK_UP:
                    if (rectangle->y < 0) {
                        continue;
                    }
                    rectangle->y -= 5;
                    break;
                case SDLK_DOWN:
                    if (rectangle->y + rectangle->h > WINDOW_HEIGHT) {
                        continue;
                    }
                    rectangle->y += 5;
                    break;
                case SDLK_LEFT:
                    if (rectangle->x < 0) {
                            continue;
                        }
                    rectangle->x -= 5;
                    break;
                case SDLK_RIGHT:
                if (rectangle->x + rectangle->w > WINDOW_WIDTH){
                        continue;
                    }
                    rectangle->x += 5;
                    break;
            }
                
        }

    }
    return 1;
}
