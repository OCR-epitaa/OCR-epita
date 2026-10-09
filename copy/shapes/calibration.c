#include "shapes.h"
#include <stdio.h>
#include "SDL2/SDL.h"

void draw_calibration(SDL_Renderer *renderer)
{
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rectangle1 = { .x = 100, .y = 100, .w = 600, .h = 400 };

    SDL_Rect rectangle = { .x = 350, .y = 250, .w = 100, .h = 100 };


    SDL_RenderDrawRect(renderer, &rectangle1);
    SDL_RenderFillRect(renderer, &rectangle);
    SDL_RenderDrawLine(renderer, 100, 100, 700, 500);
    SDL_RenderPresent(renderer);

}
