#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>

#define WINDOW_HEIGHT 900
#define WINDOW_WIDTH 1440

int init_window(SDL_Window **window, SDL_Renderer **renderer);
void terminate(SDL_Window *window, SDL_Renderer *renderer);

#endif /* ! WINDOW_H */
