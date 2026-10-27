#include "../include/images.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <err.h>


void save_image(SDL_Surface *image, const char *path)
{
    if (SDL_SaveBMP(image, path) != 0) {
        errx(1, "%s", SDL_GetError());
    }

}

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        SDL_FreeSurface(surface);
        errx(1, "%s\n", SDL_GetError());
    }
    printf("Image width: %d\n", surface->w);
    printf("Image height: %d\n", surface->h);
    printf("Bytes per pixel: %d\n", surface->format->BytesPerPixel);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    if (!texture) {
        SDL_FreeSurface(surface);
        errx(1, "%s\n", SDL_GetError());
    }
    SDL_FreeSurface(surface);

    return texture;
}