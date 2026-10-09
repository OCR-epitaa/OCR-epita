#include "images.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <err.h>

SDL_Surface *load_image(const char *path)
{
    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface ) {
        SDL_FreeSurface(surface);
        errx(1, "%s", SDL_GetError());
    }

    return surface;
}

void save_image(SDL_Surface *image, const char *path)
{
    if (SDL_SaveBMP(image, path) != 0) {
        errx(1, "%s", SDL_GetError());
    }

    // TODO
}

