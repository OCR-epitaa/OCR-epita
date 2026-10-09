#include "shapes.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include "window.h"

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    /*
     * TODO:
     * - Initialize the variables and the window
     * - Display the calibration pattern
     */

    init_window(&window, &renderer);

    SDL_Rect rectangle = { .x = 100, .y = 100, .w = 200, .h = 150 };


    while (handle_events(&rectangle) != 0)
    {
        /*
         * TODO:
         * - Clear the previous frame
         * - Render the new one
         */
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderClear(renderer);

        draw_calibration(renderer);

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);

        SDL_RenderFillRect(renderer, &rectangle);


        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 24); // ~24 FPS
    }
    terminate(window, renderer);

    return 0;
}
